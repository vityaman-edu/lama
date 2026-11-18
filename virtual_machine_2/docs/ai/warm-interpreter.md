# Warm interpreter

The decoder installs a cold handler at each instruction. On its first successful
execution, the handler performs the operation once, stores its resolved operands
in the same words, then installs the warm handler before dispatch. Conditional
jumps resolve their target on either outcome. FAIL and FAIL_KEEP terminate the
process. EOF stays shared so startup can recognize the terminator.

Warm execution assumes valid bytecode, compatible frames, live operands of the
required kinds, and enough stack space. A successful cold visit only checks
the state of that visit. Release warm handlers use the current operands and
closure on every visit and omit repeated VM validation. The C runtime and Std
bridges retain their own behavior.

The `New cold` and `New warm` columns below give the implemented operand
widths. CALL has separate local, cross-unit and Std handlers. BEGIN and
BEGIN_CLOSURE have separate handlers for zero locals and retain their
signatures for cold callers. The [four-word frame](runtime.md#stack-and-frames)
saves the caller's FP, PC, SP and unit. Cold checks recover the signature
through the return PC.
Warm operations use their own cached offsets. A capture descriptor
stores a base and an offset. Base `-1` selects the global stack, `0` selects
the current frame, and a positive base locates the live closure at `fp[base]`.
CLOSURE keeps full-width capture descriptors and the existing heap layout. See the [warm handlers](../../source/interpreter/warm/handler.cpp)
and [specialization functions](../../source/interpreter/warm/specialization.cpp).

The following design notes record the layout rationale, alternatives, and
additional checks that a checked execution mode would need.

## In-place promotion

Choose the in-memory width while parsing the serialized instruction. A site's
cold and warm layouts have the same width, measured in eight-byte
[words](opword.md). Keep that width consistent in parser emission, byte-to-word
fixups (including public entries), handler decoding, and execution. Patching
then preserves the addresses of instructions, fallthroughs, and saved returns.
The file format stays the same. Reserved words take space even in unexecuted
code and count against the aggregate decoded-size budget.

On execution, validate and resolve operands before replacing them. Write the
warm operands through a writable view of the owning text, and install the warm
handler after the operands are ready. The current singleton executor can patch
between dispatches with exclusive access. Promotion may happen on the first
successful resolution. A later execution threshold would require measuring the
cost of keeping a counter. Either policy executes the instruction's side
effects once. Code, string buffers, the unit table, and the stack backing
globals must remain stable for the lifetime of the references they supply.

Checks on immutable input can be paid once per site, for example string bounds,
symbol kind, or a direct call's declared arity. Use always-on validation for
bytecode contracts. Check stack capacity and
availability, live operand types and indices, indirect call targets and frame
state whenever they can vary. Removing a check requires an invariant covering
every later execution, including other callers, branch outcomes, and recursion.
A successful first execution establishes facts about that execution only.
For a conditional transfer with deferred checks on the target, resolve and
promote when the edge first runs. Fallthrough remains usable in the meantime.

## Control transfers, calls, and closures

A direct target points at its **instruction word**. Dispatch reads the handler
from that word, so it works when the destination is cold, warm, or promoted
later. Keep the current unit index across local jumps. A cross-unit call saves
the caller's unit index and return address, installs the callee's index, then
dispatches. END restores the saved index and address. CLOSURE resolves its
callee's unit without changing the current unit, because its capture sources
belong to the creator's context.

Use an inline `u32` unit index alongside an `i32` count in CALL and CLOSURE.
Under the [aggregate size limit](units.md), each unit uses at least an EOF word
and one string byte, so all loaded unit indices fit. The index saves bits in
operand words. A live index still uses a register and a frame saved on the
existing stack still uses a full box slot. Cold operations that need unit
metadata can obtain it as `units[unit_index]`. A separate same-unit warm CALL
can keep the current index.

The earlier two-word CALL design used an inline `i32` target offset and two
`i16` fields for argument count and target unit. The three-word proposal below
adds one word per CALL to hold a native target pointer and full-width count and
index. It removes the unit-text base lookup and its offset addition from
bytecode CALL. Keep the two-word layout as the footprint baseline when
measuring. A narrow layout needs checked representability and a full-width
path for valid counts or indices that exceed its limits.

CALL also resolves Std symbols, with Std taking precedence in the current
handler. A resolved Std site uses a distinct warm handler and caches the
typed `std_fn_some_t` bridge. Resolve fixed or variadic arity once, pass the
live arguments through the bridge with the existing GC protocol, leave the unit
index in place, and fall through. Bytecode CALL saves a VM frame and returns
to the instruction after the entire CALL. Both forms use `site + 3` for the
continuation in the proposed layout.

Keep the `BEGIN` and `BEGIN_CLOSURE` signatures available after promotion.
Cold callers and closure creators can reach a warm entry. Direct CALL can
validate against `BEGIN` once per site. CALLC must validate the actual closure
and its target/arity on each execution unless those facts are established by
another invariant. A closure can name a `BEGIN` entry with no captures. For
`BEGIN_CLOSURE`, check capture-count agreement. Top-level unit entry also
uses this contract, with the synthetic two-argument frame in
[`run.cpp`](../../source/interpreter/run.cpp). Local initialization still
requires space in the current stack.

The first warm CLOSURE can keep the existing heap layout
`[boxed word offset, unit pointer, captures...]`. It can derive the offset
from the cached target pointer and the target unit's text base when constructing
the object. To remove the offset calculation from CALLC, migrate both cold and
warm producers and all consumers together to
`[code pointer, boxed unit index, captures...]`. Audit the C runtime's
[closure operations](../../contrib/runtime/runtime.c) and
[collector](../../contrib/runtime/gc.c) before that change. The collector
skips slot 0 and scans subsequent slots. Native printing, hashing, and
comparison can observe the metadata. CALLC must read the closure on the stack
each time, using the current compiler convention `sp[n_args]`.

## Capture layout

Use full-width capture descriptors for the first implementation. The old
decoded descriptor is `[kind:i32 | index:i32]`. The file uses `u8` for kind.
The proposed signed `i28` index loses valid external global references, whose
negative operand encodes a string offset as `-index - 1`. The string table can
exceed the signed 28-bit range. Decode that expression in widened arithmetic
before checking the offset.

Two descriptors per word are worth evaluating for capture-heavy code. The four
kinds fit in two bits. A compact encoding can give external globals a separate
discriminator and encode their unsigned string offsets, with a wide layout for
unrepresentable descriptors. Choose compact or wide from representability
during parsing, before computing targets, and keep the selected width through
promotion. With two descriptors per word the total is `3 + ceil(N/2)` words
instead of the current `2 + N`. The compact split layout grows at zero and one
capture, ties at two and three, and saves space from four onwards. An odd count
leaves one unused lane. Warm descriptors can resolve global slots once and
precompute frame-relative displacements. Read all capture values anew when
creating each closure.

## Per-opcode layouts

Each bracket is one operand word. The leading handler word is implicit.
`—` means there are no operand words, and `=` repeats the layout in the column
to its left. A lone `i32` uses the first lane, with the other lane unused unless
shown. `pc*` points to an instruction word, and `box` denotes already-tagged
bits. Store these bits in a trivial 64-bit operand member and recover the box
without applying its integer constructor. Old cold records the earlier layout. New cold is emitted by
[the parser](../../source/bc/unit.cpp). New warm is installed in place. See the
[cold handler implementations](../../source/interpreter/cold/handler.cpp)
and [runtime primitives](../../contrib/runtime/runtime.c) for the behavior
each warm handler needs to preserve.

| Op | Old cold | New cold | New warm | Note |
|:--|:--|:--|:--|:--|
| `op_add` | — | = | = | No inline operand. Any tagged-integer fast path needs the live operand checks or an established type invariant. |
| `op_sub` | — | = | = | Preserve the runtime's integer and boxed-pointer cases when inlining. |
| `op_mul` | — | = | = | Keep the runtime integer contract. An inline fast path must define the handling of arithmetic range before replacing the C operation. |
| `op_div` | — | = | = | Preserve signed division. Divisor validity depends on the live operand. |
| `op_mod` | — | = | = | Preserve signed remainder. Divisor validity depends on the live operand. |
| `op_lt` | — | = | = | Signed integer comparison with dynamic operand checks. |
| `op_le` | — | = | = | Signed integer comparison, including equal operands. |
| `op_gt` | — | = | = | Signed integer comparison in left-to-right operand order. |
| `op_ge` | — | = | = | Signed integer comparison, including equal operands. |
| `op_eq` | — | = | = | Current runtime compares raw boxed values, including pointer identity. |
| `op_ne` | — | = | = | Current runtime requires integers. Preserve that distinction from EQ. |
| `op_and` | — | = | = | Both arguments are already evaluated. Return a boxed boolean using integer truthiness. |
| `op_or` | — | = | = | Both arguments are already evaluated. Return a boxed boolean using integer truthiness. |
| `op_const` | `[value:i32]` | = | `[value:box]` | Prebox the literal once, preserving sign. |
| `op_string` | `[string_offset:i32]` | = | `[str:const char*]` | Accept the pointer proposal. Validate the table reference before warming and allocate a fresh mutable string each time. |
| `op_sexp` | `[tag_offset:i32 \| n_fields:i32]` | `[tag_offset:i32] [n_fields:i32]` | `[tag_hash:box] [n_fields:i32]` | Accept the split. Cache the exact boxed `LtagHash` result. Keep the ordinary count for stack movement, ordering and allocation. |
| `op_sta` | — | = | = | Live aggregate, index and value still determine checks and behavior. Leave the stored value on the stack. |
| `op_jmp` | `[target:i32]` | = | `[target:pc*]` | Accept the pointer proposal after target and legal control-flow checks. Keep the current unit. |
| `op_end` | — | = | = | Restore the live frame, including caller unit and return address. Preserve the result. |
| `op_drop` | — | = | = | Advance `sp` after establishing that a value is available. |
| `op_dup` | — | = | = | Copy the top slot into a new slot. Depth and capacity can vary. |
| `op_swap` | — | = | = | Swap the two live slots in place after the depth check or proof. |
| `op_elem` | — | = | = | Aggregate kind and index depend on live operands. Preserve the layout differences among strings, arrays and S-expressions. |
| `op_ld_glo` | `[index:i32]` | = | `[slot:box*]` | Resolve local or imported globals to stable stack cells. Check the declaring unit's global count before forming the pointer. |
| `op_ld_loc` | `[index:i32]` | = | `[fp_disp:i32]` | Precompute `-frame::size - index` after validating the local index. The frame itself changes. |
| `op_ld_arg` | `[index:i32]` | = | `[fp_disp:i32]` | Precompute `n_args - index` only with an enforced entry signature. Otherwise keep the index and read the live frame count. |
| `op_ld_clo` | `[index:i32]` | = | `[closure_fp_disp:i32 \| capture_disp:i32]` | With a fixed arity, cache the frame offset of the closure and payload offset `2 + index`. Read the live closure every time. |
| `op_st_glo` | `[index:i32]` | = | `[slot:box*]` | Cache the writable cell, whose value may be changed by stores or updated by GC. Leave the assigned value on the stack. |
| `op_st_loc` | `[index:i32]` | = | `[fp_disp:i32]` | Same displacement as LD_LOC. Store the top value in the current frame after establishing it is available. |
| `op_st_arg` | `[index:i32]` | = | `[fp_disp:i32]` | Same signature requirement and index fallback as LD_ARG. Preserve the top value. |
| `op_st_clo` | `[index:i32]` | = | `[closure_fp_disp:i32 \| capture_disp:i32]` | Same offset requirements as LD_CLO. Store into the current closure, whose address can change across calls or GC. |
| `op_cjmp_z` | `[target:i32]` | = | `[target:pc*]` | Accept the pointer proposal. Test the current integer for zero. Fallthrough is `site + 2`. |
| `op_cjmp_nz` | `[target:i32]` | = | `[target:pc*]` | Same layout with a nonzero test. Keep unit context on either edge. |
| `op_begin` | `[n_args:i32 \| n_locals:i32]` | = | = | Retain the signature. A warm entry can specialize local initialization, including zero locals. |
| `op_begin_closure` | `[n_args:i32 \| n_locals:i32] [n_captures:i32]` | = | = | Retain both counts for later calls and closure creation. Warm fallthrough still skips both operands. |
| `op_closure` | `[target:i32 \| N:i32] [kind:i32 \| index:i32] × N` | `[target:i32] [_ \| N:i32] [kind:i32 \| index:i32] × N` | `[target:pc*] [unit_index:u32 \| N:i32] [base:i32 \| offset:i32] × N` | Accept the split header. Use wide descriptors initially. The slot denotes a resolved global stack index or a frame/payload-relative coordinate. Preserve capture order and GC rooting. |
| `op_callc` | `[n_args:i32]` | = | `[indirect:u32 \| n_args:i32]` | The site has no fixed callee. A shared pointer-based closure ABI can remove target offset arithmetic. The caller continuation is still `site + 2`. |
| `op_call` | `[target:i32 \| n_args:i32]` | `[target:i32] [_ \| n_args:i32]` | VM `[target:pc*] [unit_index:u32 \| n_args:i32]` or Std `[bridge:fn] [_ \| n_args:i32]` | Accept the extra word for the direct target. Keep the count in the same lane and use separate VM and Std handlers. |
| `op_tag` | `[tag_offset:i32 \| n_fields:i32]` | `[tag_offset:i32] [n_fields:i32]` | `[tag_hash:box] [n_fields:box]` | Same hash resolution as SEXP. Prebox the count as well, since only `Btag` uses it. Subject type, tag and arity remain live tests. |
| `op_array` | `[n_elements:i32]` | = | `[n_elements:box]` | Prebox the expected length for `Barray_patt`. Keep the live kind and length tests. |
| `op_fail` | `[line:i32 \| col:i32]` | = | = | Terminal diagnostic with the popped value and current unit name. No recurring execution to optimize. |
| `op_fail_keep` | `[line:i32 \| col:i32]` | = | = | Terminal diagnostic with the top value left in place. Preserve the source location. |
| `op_line` | `[line:i32]` | = | = | Already skips the operand. A future fallthrough shortcut must respect entry addresses and any line hooks. |
| `op_patt_str_cmp` | — | = | = | The right operand is a string. Its contents and the left operand remain dynamic. |
| `op_patt_string` | — | = | = | Test the live string tag. A nonmatching value produces false. |
| `op_patt_array` | — | = | = | Test the live array tag independently of length. |
| `op_patt_sexp` | — | = | = | Test the live S-expression tag independently of constructor and arity. |
| `op_patt_boxed` | — | = | = | The live box tag is enough. An inline predicate needs no extra operand. |
| `op_patt_unboxed` | — | = | = | Same representation test as PATT_BOXED with the opposite result. |
| `op_patt_closure` | — | = | = | Test the live closure tag. Closure entry checks belong to calls. |
| `op_barray` | `[n_elements:i32]` | = | = | Count is already immediate and drives stack movement and argument order. Allocate with the existing GC protocol each time. |
| `op_eof` | — | = | = | Keep the shared terminator. Startup checks its handler identity, and it returns control to the outer unit loop. |

The hash computed by the linked runtime can hold ten six-bit characters on
the 64-bit build, so a 32-bit lane cannot hold every tag hash. A two-word
SEXP/TAG variant could pack the 60-bit unboxed hash with a four-bit arity for
counts up to 15. That trades a word for bit extraction, reboxing and a second
layout. Use the three-word layout as the baseline for measuring this tradeoff.
