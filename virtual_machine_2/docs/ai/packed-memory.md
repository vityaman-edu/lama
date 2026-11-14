# Packed layouts and memory interpretation

Status: explanation checked against the current layouts and small temporary
Clang experiments. No source changes were made for these experiments.

## Alignment

The GCC/Clang `gnu::packed` extension minimizes member padding and reduces the
alignment of the current instruction records to one byte. For example,
`op_const_t` occupies five bytes: one opcode and one four-byte integer.

Direct access through the packed parent retains the information that a member
may be unaligned. An ordinary `int32_t*` or `int32_t&` does not retain that
contract. Taking the address of a packed integer member, or binding a reference
to it, can therefore introduce an alignment violation even when reading the
member by value is supported.

This matters for templates and macros that bind arguments by reference, including
assertion and formatting APIs. In the current
[`core/assert.hpp`](../../source/core/assert.hpp), `LAMAVM_EXPECT_B` binds each
operand using `auto&&`.

## Compiler checks

A temporary example contained a packed byte followed by an integer. With local
Clang 23.1.3 targeting ARM64, the generated LLVM IR recorded:

| Access | Load alignment |
|:--|--:|
| Direct packed member access | 1 |
| Dereference of an ordinary integer pointer cast from the byte address | 4 |
| Copy into a local integer through `memcpy` | 1 |

The packed access compiled into one `ldur` load on the default target. With
`-mstrict-align`, it compiled into byte loads and shifts. The compiler chooses
the machine instructions; it does not reconstruct the entire record on every
member access.

A separate UBSan experiment accepted direct packed member access and reported
misaligned reference binding for `auto&&` bound to the same member. These were
compiler and correctness checks, not performance benchmarks.

## Object lifetime

`reinterpret_cast` itself reads no bytes, copies no record, and starts no object
lifetime. Alignment and lifetime are separate questions.

C++20 implicit object creation supports appropriate uses of implicit-lifetime
types in allocated storage and byte arrays. The current plain record types fit
that category. This does not make every cast from an arbitrary byte buffer
valid: storage provenance, object and array lifetimes, alignment, bounds, and
valid value representations still matter.

C++23 provides `start_lifetime_as` and `start_lifetime_as_array` to express
object creation in existing storage without copying its contents. Their
alignment and storage preconditions still apply.

## Representation boundaries

Packing defines layout and alignment, not byte order. The file format uses
little-endian integers. Direct interpretation of native integer fields assumes
a matching host byte order.

Packed records can describe the disk format while aligned values describe
metadata and execution. Reading a record into a local value through `memcpy`
preserves named fields without requiring typed access to the source bytes.
The [opword proposal](opword.md) changes the execution representation more
substantially.

## Sources

- [GCC packed attribute](https://gcc.gnu.org/onlinedocs/gcc/Common-Attributes.html#index-packed)
- [GCC address-of-packed-member warning](https://gcc.gnu.org/onlinedocs/gcc/Warning-Options.html#index-Waddress-of-packed-member)
- [C++ object model](https://eel.is/c++draft/intro.object)
- [Explicit lifetime management](https://eel.is/c++draft/obj.lifetime)
- [Format specification](../../../virtual_machine/SPEC.md)
