# Deferred specification gaps

These findings were checked against the source on 2026-11-15. They are
postponed; recording them does not authorize implementation. The separate STA
implementation addresses the only explicit unsupported-opcode handler found
during this review.

The requirements below come from
[`virtual_machine/SPEC.md`](../../../virtual_machine/SPEC.md).

## Execution contract

- `CALLC` reads the closure from `sp + n_args`, while the specification places
  it on top of the operand stack. These orders differ for nonzero argument
  counts. The compiler evaluates the closure before its arguments, matching
  the current handler; reconcile the specification and compiler contract
  before changing execution. See [`op_callc`](../../source/interpreter/cold/handler.cpp)
  and `Expr.Call` translation in [`SM.ml`](../../../src/SM.ml).

## Semantic validation

- `LD_LOC` and `ST_LOC` check the physical stack bounds. The runtime does
  not track the number of locals for these checks or for operand-stack pops.
- `CLOSURE` leaves capture-count agreement to the cold `BEGIN_CLOSURE`
  handler. A specialized entry assumes the count is valid.
- `CALL` and `CLOSURE` internal targets are checked for instruction boundaries,
  but not the required function-entry kind. Inter-unit targets likewise lack
  entry-kind checks.
- `JMP` and `CJMP_*` targets are not restricted to the current function body.
  Operand-stack depths at control-flow joins are not checked.
- `END` does not enforce exactly one operand-stack value.
- `EOF` must terminate the serialized stream, but the parser does not check
  that it is outside a function body.

Relevant code: [`unit_parser`](../../source/bc/unit.cpp),
[`cold handlers`](../../source/interpreter/cold/handler.cpp), and
[`stack helpers`](../../source/interpreter/runtime/state.cpp).

## Representation differences

The current cold interpreter also differs from the specification's threaded
representation notes. These are separate from missing opcode implementations:

- String references, tag hashes, external symbols, and global references are
  resolved during execution rather than decoding or loading.
- Branch and call operands retain word offsets rather than direct instruction
  pointers.
- Closure captures remain descriptors instead of expanding into load
  instructions; closure objects include a unit-context slot before captures.
- `BEGIN_CLOSURE` retains its capture-count operand.
- `LINE` remains a no-op instruction instead of being removed in non-debug
  builds; it does not report line markers.

See [`decode`](../../source/interpreter/decode.cpp) and the
[cold interpreter guide](threaded-code.md). These representation differences
need a separate design decision before implementation.
