# Bytecode safety campaign

Review one cold opcode at a time. The goal is for any serialized `.bc` input,
including imported units, to either execute with the specified behavior or fail
through a controlled VM error when it violates a precondition. Invalid bytecode
must never trigger an out-of-bounds access, corrupt VM state, dispatch through
an invalid pointer, or cause other undefined behavior. The same safety contract
applies to release builds.

Start with the opcode's contract in the
[bytecode specification](../../../virtual_machine/SPEC.md) and its encoding in
[`bc::opcode_t`](../../source/bc/layout.hpp). Follow it through the
[parser](../../source/bc/unit.cpp), [loader](../../source/bc/units.cpp),
[threaded decoder](../../source/interpreter/decode.cpp), and
[cold handler](../../source/interpreter/cold/handler.cpp). Include any helpers
that consume its inputs, including [stack and frame operations](../../source/interpreter/runtime/state.cpp),
the [C runtime](../../contrib/runtime/runtime.c), and the
[collector](../../contrib/runtime/gc.c).

The [parser](bytecode.md) already checks the encoded instruction stream and
converts local code targets only at instruction boundaries. A boundary can still
be the wrong kind of entry or belong to another function. The
[deferred defects](deferred-defects.md#semantic-validation) describe known gaps
to revisit during the reviews.

## Per-opcode review

1. State the operand layout, stack effect, and legal entry and successor states.
   Include each branch outcome and any differences among local, imported,
   closure, and native execution paths where applicable.
2. Identify the validation or invariant that establishes each precondition.
   Locate it before the first dependent operation, including pointer arithmetic,
   indexing, span construction, or dispatch. Check whether it holds in release
   builds and for every way control can reach the instruction. `LAMAVM_ASSERT`
   checks are disabled under `NDEBUG`. See [assertion definitions](../../source/core/assert.hpp).
3. Record missing preconditions and their consequences. For each confirmed
   finding, name the opcode, point to the source, and state the required behavior.
   Note related opcodes where a shared helper or invariant needs its own review.
4. When checking a fix, cover the failing boundary and a nearby valid case. For
   malformed bytecode, verify that the VM handles the intended violation through
   a controlled error. Treat signal termination and sanitizer findings as
   failures. Confirm that the test reaches the validation under review. Check
   release behavior and use the [sanitized build](tooling.md) when testing for
   invalid accesses or undefined behavior.

### Preconditions to consider

- **Stack and frames.** Check that required operands belong to the current
  frame's live operand range and that pushes, frame setup, and local allocation
  fit. Check argument, local, and capture indices against their own bounds. A
  slot can be inside the backing stack and still belong to a different frame or
  to VM metadata.
- **Operands and references.** Check counts, signedness, and arithmetic ranges
  before computing sizes or offsets. String offsets must stay within the owning
  string table. Global indices must stay within the declaring unit's globals,
  including after resolving an imported symbol.
- **Control flow.** Check legal fallthrough and jump destinations, function
  ownership, entry kind and signature, closure metadata, and return-frame
  validity. Include unit initialization and transitions between units when
  they supply or consume the same invariants.
- **Live values and runtime calls.** Check value kinds, aggregate index bounds,
  arithmetic domains, native call contracts, and GC visibility or lifetime across
  allocation. Follow the checks through the runtime primitive that actually
  consumes each value.

## Warming

For each reviewed opcode, record which guarantees come from loading or immutable
site data and which depend on the current execution. The
[warm interpreter](warm-interpreter.md) can reuse a validated immutable fact
after promotion. Stack availability, live value kinds, aggregate indices, and
indirect targets still need checks on each execution unless an invariant covers
every execution of the site.

## Review record

Use `bc::opcode_t` as the opcode inventory. Add an entry after reviewing an
opcode, with its source references, established guarantees, open findings, and
verification status. If the review finds no issue, record the guarantees that
support that conclusion. Record shared invariants once and reference them from
the opcodes that rely on them.

No opcodes have been reviewed as part of this campaign yet.
