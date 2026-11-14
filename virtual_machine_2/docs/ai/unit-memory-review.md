# Memory review of unit::parse

Status checked on 2026-11-14 against
[`source/bc/unit.cpp`](../../source/bc/unit.cpp). This review covered the unit
parser and its section views. A systematic review of `interpreter/loop.cpp`
was postponed.

## Findings and current status

| Finding | Status |
|:--|:--|
| Assertion macros bind ordinary references to unaligned packed fields | TODO |
| Section pointers and spans are constructed before section bounds are established | TODO |
| Public-symbol name offset equal to string-table size is accepted | Fixed by changing the upper bound to `<` |
| Function target equal to code-section size is accepted | Fixed by changing the upper bound to `<` |
| Public global indices are compared against code-section size | Still present; changing the comparison operator does not change the meaning of the bound |
| Vector overload lacks the file overload's size limit | A debug-only `LAMAVM_ASSERT_B(arena.size(), <, max_unit_size)` was added |

The size assertion is absent under `NDEBUG`. The file overload uses an always-on
`LAMAVM_EXPECT_B` check with `<=`. The vector overload still narrows code-section
size to `int32_t`; the added assertion does not establish that bound in release
builds.

## Packed field references

Imports start after the variable-length string table. Their integer fields
need not be four-byte aligned. Public symbols have a nine-byte stride, so even
an aligned first entry does not align subsequent entries.

`LAMAVM_EXPECT_BB` expands into `LAMAVM_EXPECT_B`, whose `auto&&` operands bind
ordinary references to these fields. This is the alignment issue, rather than
the packed parent reference itself. See [packed memory](packed-memory.md).

## Section bounds

`parse` calls `code_section()` before validating the computed code offset.
That accessor uses the declared string-table length and record counts to build
views and form pointers. A malformed header can therefore cause invalid range
construction, out-of-bounds pointer arithmetic, and size subtraction wraparound
before the intended rejection.

For example, a 24-byte file containing only a header can declare a one-byte
string table. The computed code offset is 25, and the code-section size
calculation attempts `24 - 25`.

## Symbol target meaning

The [format specification](../../../virtual_machine/SPEC.md) gives the public
symbol target two meanings:

- Function: byte offset into code.
- Global: global variable index.

The current check applies the code-size bound to both kinds. This is independent
of the corrected one-past-end comparison.

## Reproduction evidence

A temporary harness compiled the actual `unit.cpp` with ASan, UBSan, additional
unsigned-integer-overflow checking, and recovery disabled. Results before the
comparison fixes were:

| Fixture | Observed result |
|:--|:--|
| Minimal unit | Accepted |
| One-byte string table followed by an import | UBSan: misaligned reference binding in the import check |
| Four-byte string table followed by two public symbols | UBSan: misaligned reference binding in the second symbol check |
| Four-byte string table, public name offset 4 | Accepted |
| One-byte code section, function target 1 | Accepted |
| Zero globals, public global index 0 | Accepted |
| Sixteen globals, public global index 8, one-byte code section | Rejected against the code-size bound |
| Header only, declared string-table size 1 | Unsigned-overflow sanitizer: `24 - 25` |

Unsigned subtraction wraparound is not itself undefined behavior. The additional
sanitizer exposed the size arithmetic; invalid ranges and pointer arithmetic
are the separate correctness problem.

The comparison changes and added assertion were inspected afterward. The full
harness was not rerun after those edits. No multi-gigabyte fixture was allocated
to exercise the narrowing concern.

## Sources

- [C++ pointer arithmetic](https://eel.is/c++draft/expr.add)
- [Span construction](https://eel.is/c++draft/span.cons)
