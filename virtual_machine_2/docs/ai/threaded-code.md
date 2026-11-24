# Cold threaded interpreter

The execution path is implemented in
[`interpreter/decode.cpp`](../../source/interpreter/decode.cpp),
[`interpreter/cold/handler.cpp`](../../source/interpreter/cold/handler.cpp), and
[`interpreter/run.cpp`](../../source/interpreter/run.cpp).

## Handler decoding

Before any unit executes, `decode(span<op::word_t>)` replaces handler IDs with
cold handler function pointers in place. It skips operand words according to
instruction layout, leaving integer operands unchanged. Decode is a one-time
transition; its input must still contain IDs, not already installed pointers.

IDs wider than the bytecode opcode type fail with an overflow diagnostic.
Unknown handler IDs are rejected before execution, including in unreachable
code. Handler decoding assumes parser-produced layout and does not add operand,
stack, or control-flow validation.

All 52 opcode forms have handler implementations, including STA, aggregate
operations, closures, and pattern predicates. LINE consumes its operand and
has no execution effect. Handler coverage does not imply complete specification
conformance; see [deferred defects](deferred-defects.md).

STA pops the value, index, and aggregate, calls the non-allocating runtime
`Bsta`, and pushes the stored value. Its stack effect is three values to one.

TAG occupies three words with one operand each for `tag_offset` and `n_fields`. It resolves the tag in the current unit with `LtagHash`, consumes
one value, and pushes the boxed result of `Btag(value, tag_hash, BOX(n_fields))`.
The result is true only for an S-expression with both the expected tag and
arity. The operation does not allocate and leaves stack depth unchanged.

`bc::opcode_t` in `source/bc/layout.hpp` defines the order for opcode switch
cases, handler declarations and definitions, and packed-layout declarations
and formatters. Keep shared case bodies only across adjacent cases in that
order. Declare and define every cold handler, including unimplemented
instructions. Any future unimplemented handler definitions call
`fatalf("unsupported {}", bc::opcode_t::op_xxx)` when executed.

## Dispatch contract

At handler entry, `pc` points at the handler word. The
[instruction layout](opword.md#interpreter-layouts) gives access to the
operands through `op_xxx_t::reinterpret(pc)`. The cold handler passes that
same `pc` to specialization before changing it.

`LAMAVM_DISPATCH(op_xxx_t)` adds `word_count<op_xxx_t>()` to `pc` and then
dispatches. The count includes the handler word. `LAMAVM_DISPATCH_JUMP()`
dispatches through the current `pc`. Jumps choose a target or a conditional
fallthrough explicitly. CLOSURE advances by `word_count(view)`, which includes
the capture tail, and dispatches through `LAMAVM_DISPATCH_JUMP()`.

Handlers have the parameterless `void()` signature and access singleton
[runtime state](runtime.md). Dispatch uses `LAMAVM_MUSTTAIL`, defined as
`[[clang::musttail]]` in `core/compiler.hpp`. Matching signatures and required
tail calls prevent one native stack frame per executed instruction.

## Calls and unit context

Bytecode CALL and CALLC save the argument count, next handler address, frame
and stack pointers, and caller unit context. The continuation is
`pc + word_count<Layout>()`. A nonnegative cold CALL target is a word offset
in the current unit. A negative target names a Std or public function. A Std
call uses its native bridge and falls through in the current unit. A public
function call selects the declaring unit. END specializes at the current
instruction and then restores the caller state for dispatch at the saved
return address.

Global loads and stores resolve negative operands as public globals and use the
declaring unit's global base. STRING uses the current unit's string table.
Unit context therefore matters independently of whether an instruction is warm.

EOF returns from the handler chain. The outer initialization loop then starts
the next unit. Once all bodies finish, `run` releases the VM stack and returns.
An atomic guard permits one runtime load per process.

Current bounds-checking gaps are recorded in [deferred defects](deferred-defects.md).
The [warm interpreter](warm-interpreter.md) specializes each returning cold
instruction before its next dispatch.
