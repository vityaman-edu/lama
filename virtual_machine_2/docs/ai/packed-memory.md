# Packed layouts and memory interpretation

[`bc/layout.hpp`](../../source/bc/layout.hpp) describes packed disk records.
The current [parser](bytecode.md) reads scalar values from a stream rather than
borrowing packed instruction references from a file buffer. The following
constraints still apply when working with packed records or raw storage.

## Alignment

The GCC/Clang `gnu::packed` extension minimizes padding and reduces the alignment
of these instruction records to one byte. `op_const_t` occupies five bytes:
one opcode byte followed by an int32 operand.

Direct access through a packed parent tells the compiler that a field may be
unaligned. An ordinary `int32_t*` or `int32_t&` loses that information. Taking
an address or binding a reference to a packed member can therefore violate
alignment even when reading the member by value is supported.

This matters for assertion and formatting templates. In
[`core/assert.hpp`](../../source/core/assert.hpp), `LAMAVM_EXPECT_B` binds its
operands using `auto&&`. Copy a possibly unaligned field into an aligned value
before passing it to an interface that binds ordinary references.

The compiler chooses loads appropriate for the target and its alignment options;
packed access does not imply reconstructing a whole record for every field.

## Object lifetime and storage

`reinterpret_cast` reads no bytes, copies no record, and starts no object lifetime.
Alignment, bounds, lifetime, and valid representations are separate requirements.
C++20 implicit creation of implicit-lifetime objects covers appropriate allocated
storage and byte-array operations, not every arbitrary cast. C++23's
`start_lifetime_as` APIs also retain storage and alignment preconditions.

[`ctype`](../../source/core/c.hpp) requires aggregate, standard-layout, trivial
record properties. It does not imply byte alignment or validate a buffer's
contents. Copying an object's representation also does not copy any storage it
references; a copied view still refers to its original data.

The [interpreter layouts](opword.md#interpreter-layouts) use direct casts over
the `op::word_t` buffer as a deliberate implementation choice. Their `ctype`
assertions check struct properties. The backing objects remain `op::word_t`
elements, so object lifetime remains a separate C++ requirement for member
access through the cast.

Keep allocated capacity separate from written size. `vector::reserve` leaves
size unchanged; spare capacity is not an element range for appending bytes by
raw writes. An overwrite allocation can avoid initial byte-value initialization,
but still needs explicit size, bounds, and lifetime management. Small `memcpy`
operations may compile into loads and stores rather than library calls.

## Representation boundaries

Packing does not define byte order. The serialized format is little-endian;
direct native-field interpretation needs matching endianness. Aligned execution
[words](opword.md) and ordinary metadata values have different layout contracts
from packed disk records.

## References

- [GCC packed attribute](https://gcc.gnu.org/onlinedocs/gcc/Common-Attributes.html#index-packed)
- [GCC packed-member address warning](https://gcc.gnu.org/onlinedocs/gcc/Warning-Options.html#index-Waddress-of-packed-member)
- [C++ object model](https://eel.is/c++draft/intro.object)
- [Explicit lifetime management](https://eel.is/c++draft/obj.lifetime)
- [Vector capacity and resizing](https://eel.is/c++draft/vector.capacity)
