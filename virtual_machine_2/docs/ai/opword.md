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
| `as_target` | Instruction pointer for warm control transfers |
| `as_i32` | Two signed 32-bit operands, `arg0` and `arg1` |
| `as_box_bits` | Already-tagged literal, hash, or pattern count |
| `as_slot` | Stable global stack cell |
| `as_capture` | Base and offset for a resolved capture |
| `as_call` | Unit index and count in a resolved CALL or CLOSURE |
| `as_std_fn` | Resolved Std bridge |
| `as_str` | String-table pointer for warm STRING |

An instruction starts with one handler word, followed by operand words. The parser packs BEGIN and source coordinates into two 32-bit lanes.
SEXP, TAG, CALL, and CLOSURE reserve two header operands for warming. CLOSURE adds one word per capture designation.

| Instruction | Serialized size | Decoded size |
|:--|--:|--:|
| DUP | 1 byte | 1 word / 8 bytes |
| CONST | 5 bytes | 2 words / 16 bytes |
| JMP | 5 bytes | 2 words / 16 bytes |
| BEGIN | 9 bytes | 2 words / 16 bytes |
| CALL | 9 bytes | 3 words / 24 bytes |

Handler decoding advances by the following total word counts, including the
handler word, and leaves operand words unchanged:

| Instructions | Total words |
|:--|--:|
| BINOP forms, STA, END, DROP, DUP, SWAP, ELEM, PATT forms, EOF | 1 |
| CONST, STRING, JMP, LD/ST forms, CJMP forms, BEGIN, CALLC, ARRAY, FAIL, FAIL_KEEP, LINE, BARRAY | 2 |
| SEXP, TAG, CALL, BEGIN_CLOSURE | 3 |
| CLOSURE | 3 + n_captured |

For CLOSURE, `n_captured` is `arg1` of the second operand word. Each capture
designation occupies one additional word. These counts describe the parsed
layout; unsupported handlers are still rejected during handler decoding.

The union is a storage representation. Parsing and handler decoding establish
which member consumers read. The source checks `ctype<word_t>`, equality of
handler ID and function pointer sizes, and equality of word and
`uintptr_t` sizes.

## Operand coordinates

Cold operands remain integers after handler decoding. Local code targets are
word offsets, string operands are byte offsets into the current unit's string
table, and nonnegative global operands are indices within the current unit.
A negative external operand `x` names a symbol at string offset `-x - 1`.
[The cold handlers](threaded-code.md) resolve imported calls and globals at runtime.

The [warm interpreter](warm-interpreter.md) fills the reserved words with
resolved values. Bytecode CALL stores an instruction pointer, unit index,
and argument count. Moving text after promotion would require
[relocation](relocation.md).

## Ownership and performance

Runtime unit contexts borrow spans from owning bytecode units. Their code and
string buffers must stay alive and stable while execution uses them. Copying
backing arrays does not rebase views, saved instruction pointers, or other roots.

Words increase instruction-stream size while removing repeated packed decoding.
Handler pointers still require indirect control transfers. A speedup over a
switch or packed-byte execution is not established by the representation alone;
measure load/decode time separately from execution time.
