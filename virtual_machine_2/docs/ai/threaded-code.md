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
Unknown and unsupported opcodes are rejected before execution, including in
unreachable code. Recognized but unimplemented instructions fail with
`fatalf("unsupported {}", opcode)`. Handler decoding assumes parser-produced
layout and does not add operand, stack, or control-flow validation.

Supported operations are the 13 arithmetic/comparison/boolean BINOP forms,
CONST, STRING, SEXP, DROP, DUP, SWAP, LD/ST for globals, locals and arguments, JMP,
CJMP_Z, CJMP_NZ, CALL, BEGIN, END, TAG, LINE, and EOF. LINE consumes its operand and
has no execution effect. Other opcodes remain unsupported.

TAG occupies two words: the handler and packed `tag_offset`, `n_fields`
operands. It resolves the tag in the current unit with `LtagHash`, consumes
one value, and pushes the boxed result of `Btag(value, tag_hash, BOX(n_fields))`.
The result is true only for an S-expression with both the expected tag and
arity. The operation does not allocate and leaves stack depth unchanged.

`bc::opcode_t` in `source/bc/layout.hpp` defines the order for opcode switch
cases, handler declarations and definitions, and packed-layout declarations
and formatters. Keep shared case bodies only across adjacent cases in that
order. Declare and define every cold handler, including unimplemented
instructions. Unimplemented handler definitions call
`fatalf("unsupported {}", bc::opcode_t::op_xxx)`. The decoding switch also
rejects these instructions before execution.

## Dispatch contract

At handler entry, `pc` points at the handler word. Operand reads advance it to
an operand word. `LAMAVM_DISPATCH` increments `pc` and dispatches;
`LAMAVM_DISPATCH_JUMP` dispatches through the current `pc` without incrementing.
Taken jumps and both conditional outcomes arrange `pc` at the next handler.

Handlers have the parameterless `void()` signature and access singleton
[runtime state](runtime.md). Dispatch uses `LAMAVM_MUSTTAIL`, defined as
`[[clang::musttail]]` in `core/compiler.hpp`. Matching signatures and required
tail calls prevent one native stack frame per executed instruction.

## Calls and unit context

CALL saves the next handler address, frame/stack pointers, argument count, and
caller unit context. A nonnegative target is a word offset in the current unit;
a negative target resolves a public function and selects its declaring unit.
END restores the caller state and dispatches at the saved return address.

Global loads and stores resolve negative operands as public globals and use the
declaring unit's global base. STRING uses the current unit's string table.
Unit context therefore matters independently of whether an instruction is warm.

EOF returns from the handler chain. The outer initialization loop then starts
the next unit; after all bodies finish, `run` exits the process with status zero.
An atomic guard permits only one runtime load per process.

Cold execution does not provide comprehensive bounds checking. Per-instruction
promotion and the narrower inline CALL layout are described in the
[unimplemented warm design](warm-interpreter.md).
