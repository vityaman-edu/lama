# Pointer relocation by memory regions

Status: proposed shared mechanism for linking resolved modules and, potentially,
moving code or data during execution. Runtime movement has not been implemented
or established as a requirement.

## Pointer enumeration

The text is decodable from its entry point, which is an instruction boundary.
The handler identifies the instruction layout, including operand count and which
operands are pointers. Traversal advances over both the handler and its operands.

This provides exact pointer enumeration. Relocation does not scan arbitrary words
and guess pointer identity from their numeric values. Separate relocation records
or pointer bitmaps are optional for locating pointer operands in resolved code;
the existing instruction layout information can supply them.

A future specialized or JIT handler must preserve a known operand layout or make
its layout available to this traversal. Reachable code with multiple entry points
or non-instruction data needs an equally explicit traversal contract.

## Region mapping

Each mapping describes a source range and its destination:

```text
[old_begin, old_end) -> new_begin
```

For a known pointer `p` inside the source range, the translated pointer is:

```text
new_begin + (p - old_begin)
```

Offsets here are byte offsets. Pointers outside all moved source ranges remain
unchanged. Text pointers and string pointers can use the same address mapper;
their enumeration remains instruction-specific.

For concatenation, each source module contributes its old text and string ranges
with destinations in the new storage. Splitting or reordering can be expressed
with more source ranges, provided each piece preserves its internal layout.

This operation does not resolve an unknown external symbol. Symbol resolution
must first establish a target identity or address.

## Address lookup

For disjoint source ranges prepared before relocation, start with a vector
sorted by `old_begin`:

1. Find the last range whose beginning is not greater than `p`.
2. Check that `p` is below that range's end.
3. Translate using that range, or leave the pointer unchanged if there is no match.

`upper_bound` supports the predecessor search in O(log R), where R is the number
of ranges. For a handful of ranges, a linear scan is a reasonable baseline.
An interval tree adds little when ranges are disjoint and immutable during a pass.

If measurement identifies lookup as a bottleneck with many ranges, a page-indexed
or radix structure can narrow candidates using address bits. It costs additional
memory and construction work. Arbitrarily small ranges can share a page, so page
membership alone need not identify the exact mapping.

On the target x86-64 platform, numeric object addresses represented by
`uintptr_t` are a practical basis for indexing. Built-in relational comparisons
between unrelated allocation pointers do not give a portable address ordering.

## Mapping constraints

- Source ranges are disjoint, and destination storage has the required size,
  alignment, and lifetime.
- Each reference is translated according to its original target, exactly once
  per movement. A new address must not accidentally be treated as another old
  address in the same pass.
- Moving a piece preserves its bytes and internal offsets. Changing instruction
  sizes requires finer target mapping.
- A string or instruction cannot be split into noncontiguous pieces while its
  consumers still rely on contiguous storage and ordinary pointer arithmetic.
- One-past-end pointers need an explicit convention. Half-open ranges exclude
  them, and adjacent ranges can make their numeric addresses ambiguous.

## Runtime references

The decoder covers references inside text. Moving live storage also requires
visiting every other reference to it, including:

- The current instruction pointer.
- Saved return addresses and other pointer-bearing VM stack slots.
- Code pointers in heap closures.
- Exported entry points and runtime tables.
- Relevant pointers held by native handlers or generated code.

Runtime movement therefore needs a safepoint and a complete root-enumeration
contract. Scanning the VM stack alone is insufficient. A mistaken rewrite of an
integer that merely resembles an address would corrupt the value; pointer
identification outside the text also needs to be exact.

The reference VM stores a code target in the closure payload in
[`op_closure`](../../../virtual_machine/ops.c), illustrating a reference outside
the text and stack.

Region mapping, instruction-aware pointer visitation, and runtime root visitation
are separate responsibilities that can share the same address-translation helper.
JIT compilation and dynamic loading can initially use stable new allocations;
neither requires moving already executing code.

## Sources

- [Binary-search complexity](https://eel.is/c++draft/upper.bound)
- [C++ pointer comparisons](https://eel.is/c++draft/expr.rel)
- [Boehm collector address-to-page lookup](https://www.hboehm.info/gc/gcdescr.html)
- [JITLink relocation records and fixup passes](https://llvm.org/docs/JITLink.html)
