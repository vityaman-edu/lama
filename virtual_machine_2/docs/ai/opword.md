# Aligned execution words

Status: proposed execution representation. The interpreter currently executes
packed instructions through the decoder described in
[execution-decode.md](execution-decode.md).

## Proposed layout

The proposed `opword` is an aligned union occupying eight bytes on the target
platform. A cell contains one of:

- An opcode-handler function pointer.
- A pointer to another code word.
- A string pointer.
- A signed or unsigned 64-bit operand.
- Two 32-bit operands.

An instruction occupies a variable number of fixed-size words. DUP and SWAP
need only a handler word. JMP has a handler word followed by a target word.
Two 32-bit operands can share one operand word when their runtime meaning fits.

| Instruction | Packed format | Proposed execution words |
|:--|--:|--:|
| DUP | 1 byte | 8 bytes |
| CONST | 5 bytes | 16 bytes |
| JMP | 5 bytes | 16 bytes |
| BEGIN with two 32-bit operands | 9 bytes | 16 bytes |

The union is a storage representation, not permission to read any member
interchangeably. Decoding and linking must establish the member that execution
will read. Eight-byte size and alignment are target assumptions to verify.

## Dispatch

The function is an opcode handler; `handler_t` was suggested as a possible name.
Storing its address removes the opcode-to-handler lookup. It does not eliminate
the indirect control transfer or guarantee a speedup over a switch, which may
already compile into a jump table.

The dispatch mechanism remains an open decision:

- A central loop calls a handler and resumes after it returns.
- Handlers transfer to the next handler using guaranteed tail calls.

Ordinary recursive handler calls accumulate native stack frames. The reference
VM uses a handler signature carrying `ip`, `sp`, and `bp`, and explicit
`musttail` calls in its dispatch macros. A no-argument handler signature would
require VM state to be available elsewhere.

Dispatch pointers may later be patched to specialized or generated handlers.
Such replacements need a compatible calling convention, operand layout, and
continuation contract. They must also preserve the decoder's ability to identify
the instruction layout; see [relocation](relocation.md).

## Operand conversion

The runtime representation can do more than align the disk operands:

- Translate branch and call byte offsets into code pointers.
- Resolve string offsets into stable string pointers.
- Resolve global references.
- Precompute runtime values that would otherwise be reconstructed repeatedly.

For example, the reference VM computes a SEXP tag hash while decoding. Its hash
uses a runtime-width operand. Keeping a 32-bit tag offset and field count in one
word is a different choice; the required runtime tag representation must be
settled before assuming both runtime operands fit together.

## Performance and ownership

Packed bytecode reduces the data read for the instruction stream. Aligned words
increase that footprint while reducing repeated decoding and address resolution.
The stream is data consumed by native handlers; distinguish its data-cache
footprint from the instruction-cache footprint of the handlers themselves.

Neither representation was benchmarked against the other. Small hot loops and
larger programs may expose different costs. Measure load/link time separately
from execution time.

A program must own all referenced code and string storage. Internal pointers
make an ordinary copy of its backing arrays insufficient. Stable allocation is
the initial proposal; pointer relocation is discussed separately.

## References

- [Reference instruction union](../../../virtual_machine/insn.h)
- [Reference dispatch macros](../../../virtual_machine/ops.c)
- [Reference decoder](../../../virtual_machine/converter.c)
- [Clang musttail](https://clang.llvm.org/docs/AttributeReference.html#musttail)
- [GCC switch jump tables](https://gcc.gnu.org/onlinedocs/gcc/Code-Gen-Options.html#index-fno-jump-tables)
- [C++ unions](https://eel.is/c++draft/class.union)
