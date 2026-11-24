# Pointer relocation by memory regions

Status: future mechanism. Cold targets are integer coordinates. Warm jumps,
CALL, and CLOSURE cache direct instruction pointers. Warm STRING caches a
string-table pointer. These operands depend on stable backing allocations.
See the [warm layouts](../../source/interpreter/warm/layout.hpp).

## Pointer enumeration

A traversal can enumerate exact pointer locations when every handler has a known
instruction layout: visit pointer operands and skip other operand words.
It must not guess pointer identity from numeric values. Future specialized or
JIT handlers must preserve a known layout or expose equivalent metadata.
The current handler decoder is not a pointer-relocation visitor.

Separate relocation records or pointer bitmaps are optional for locating resolved
pointer operands when layout information already identifies them. Symbolic
fixups still need a target identity before address translation is possible.

## Region mapping

Each mapping describes a source range and its destination:

```text
[old_begin, old_end) -> new_begin
p -> new_begin + (p - old_begin)
```

Offsets are in bytes. A pointer outside all moved ranges remains unchanged.
For concatenation, each source unit contributes its text and string ranges.
Reordering or splitting uses more ranges, provided each piece retains its layout.

For disjoint source ranges, a vector sorted by `old_begin` supports predecessor
lookup with `upper_bound` in O(log R), followed by a check against `old_end`.
A linear scan is a reasonable starting point for a few ranges. On the target
x86-64 platform, `uintptr_t` values can provide numeric address ordering;
built-in relational comparison of unrelated allocation pointers is not a
portable substitute.

## Mapping constraints

- Source ranges are disjoint; destinations have sufficient size, alignment,
  and lifetime.
- Translate each reference from its original target exactly once. A new address
  must not be mistaken for another old address during the same pass.
- Byte rebasing preserves internal offsets. Changing instruction sizes requires
  a mapping of individual target positions.
- Strings and instructions must remain contiguous where consumers require it.
- One-past-end pointers need an explicit convention: half-open ranges exclude
  them, and adjacent ranges can make the numeric address ambiguous.

## References outside text

Moving live storage also requires visiting the current instruction pointer,
saved return addresses, unit-context spans, closure unit pointers, exported
entry points, and pointers retained by native handlers or generated code.
VM2's current heap closures store code offsets relative to their unit text.
The reference VM's [closure handler](../../../virtual_machine/ops.c) illustrates
code references outside text.

Runtime movement needs a safepoint and complete root enumeration. Scanning the
VM stack alone is insufficient, and treating an integer as a pointer corrupts
its value. Region mapping, instruction traversal, and runtime root enumeration
are separate responsibilities that can share the address translator.

## References

- [Binary-search complexity](https://eel.is/c++draft/upper.bound)
- [C++ pointer comparisons](https://eel.is/c++draft/expr.rel)
- [JITLink fixup passes](https://llvm.org/docs/JITLink.html)
