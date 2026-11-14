# Writable byte buffer

Status: design postponed. The current declaration is
[`source/core/arena.hpp`](../../source/core/arena.hpp). No buffer implementation
was added during this discussion.

## Requirements and API choices

- Allocate enough storage ahead of writing.
- Append packed records, primitive values, strings, and memory areas.
- Express copying writes through `memcpy`.
- Keep allocated capacity separate from the number of written bytes.
- Read and write operations are unsafe APIs. The intended implementation has
  some `LAMAVM_ASSERT` checks, rather than comprehensive runtime validation.
- Typed record operations use `ctype` from
  [`core/c.hpp`](../../source/core/c.hpp). Explicit operations for primitive
  types are planned.
- Capacity is mutable so ownership can be transferred by move assignment.

## Storage choices discussed

| Storage | Consequence |
|:--|:--|
| `vector<byte>` resized ahead of writing, with a separate cursor | Simple ownership, but bytes are initialized before being overwritten |
| `make_unique_for_overwrite<byte[]>(capacity)`, with size and capacity | Byte values are not initialized; capacity management belongs to the buffer |

`vector::reserve` does not change the vector's size or expose its spare capacity
as an element range. C++23 has no standard vector operation that commits bytes
written directly into spare capacity as newly appended elements.

Small `memcpy` operations may compile into individual loads and stores. Requiring
an actual call to the library function is not a performance goal. No buffer
benchmark was performed.

## Open questions

- Is capacity fixed after allocation, or can writes grow the allocation?
- What does parameterless `write<T>()` do to written size and initialization?
- What are the precise preconditions of typed pointer access?
- How are string contents written, and is a terminating zero included?
- What happens when source and destination areas overlap?

`ctype` does not imply byte alignment. An ordinary aggregate containing an
integer can satisfy it. Packed-record pointers and ordinary primitive pointers
therefore need distinct alignment contracts; see [packed memory](packed-memory.md).

The earlier template constrained only by `is_trivially_copyable` also admitted
`string_view`: copying that object's representation copies the view rather than
its characters. The subsequent `ctype` constraint changes that overload set;
explicit string-content operations remain a separate API question.

## Sources

- [Vector capacity and resizing](https://eel.is/c++draft/vector.capacity)
- [Unique pointer creation and overwrite allocation](https://eel.is/c++draft/unique.ptr.create)
