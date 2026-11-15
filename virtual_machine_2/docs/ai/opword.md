# Aligned execution words

[`op::word_t`](../../source/op/word.hpp) is the current execution representation.
Serialized bytecode is expanded into words by [the parser](bytecode.md), then
handler IDs are replaced by function pointers before execution.

## Layout

On the configured 64-bit target, a word occupies eight bytes. The union exposes:

| Member | Meaning |
|:--|:--|
| `as_handler_id` | Pointer-sized unsigned handler ID, initially the opcode value |
| `as_handler_fn` | Parameterless `void (*)()` handler |
| `as_target` | Word pointer, available but unused by current cold operands |
| `as_i32` | Two signed 32-bit operands, `arg0` and `arg1` |

An instruction starts with one handler word, followed by operand words. Each
pair of serialized int32 operands shares a word; a single operand uses `arg0`
and initializes `arg1` to zero. CLOSURE adds one word per capture designation.

| Instruction | Serialized size | Decoded size |
|:--|--:|--:|
| DUP | 1 byte | 1 word / 8 bytes |
| CONST | 5 bytes | 2 words / 16 bytes |
| JMP | 5 bytes | 2 words / 16 bytes |
| BEGIN | 9 bytes | 2 words / 16 bytes |
| CALL | 9 bytes | 2 words / 16 bytes |

Handler decoding advances by the following total word counts, including the
handler word, and leaves operand words unchanged:

| Instructions | Total words |
|:--|--:|
| BINOP forms, STA, END, DROP, DUP, SWAP, ELEM, PATT forms, EOF | 1 |
| CONST, STRING, SEXP, JMP, LD/ST forms, CJMP forms, BEGIN, CALLC, CALL, TAG, ARRAY, FAIL, FAIL_KEEP, LINE, BARRAY | 2 |
| BEGIN_CLOSURE | 3 |
| CLOSURE | 2 + n_captured |

For CLOSURE, `n_captured` is `arg1` of the first operand word. Each capture
designation occupies one additional word. These counts describe the parsed
layout; unsupported handlers are still rejected during handler decoding.

The union is a storage representation. Parsing and handler decoding establish
which member consumers read; its existence does not make arbitrary member
reinterpretation valid. The source checks `ctype<word_t>` and equal sizes of
handler IDs and function pointers; `ctype` alone does not establish alignment.

## Operand coordinates

Cold operands remain integers after handler decoding. Local code targets are
word offsets, string operands are byte offsets into the current unit's string
table, and nonnegative global operands are indices within the current unit.
A negative external operand `x` names a symbol at string offset `-x - 1`.
[The cold handlers](threaded-code.md) resolve imported calls and globals at runtime.

The [warm CALL design](warm-interpreter.md) also retains integer coordinates,
with a unit index and word offset. Resolved native pointers in operands and
[relocation](relocation.md) remain future possibilities.

## Ownership and performance

Runtime unit contexts borrow spans from owning bytecode units. Their code and
string buffers must stay alive and stable while execution uses them. Copying
backing arrays does not rebase views, saved instruction pointers, or other roots.

Words increase instruction-stream size while removing repeated packed decoding.
Handler pointers still require indirect control transfers. A speedup over a
switch or packed-byte execution is not established by the representation alone;
measure load/decode time separately from execution time.
