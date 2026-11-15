# Bytecode parsing

The serialized format is defined in
[the reference specification](../../../virtual_machine/SPEC.md).
[`bc/layout.hpp`](../../source/bc/layout.hpp) declares its packed records and
opcode values. Execution uses [aligned words](opword.md).

## Owning unit

[`bc::unit`](../../source/bc/unit.hpp) owns a character string table, a vector of
instruction words, an import-name vector, and a public-symbol map, and records
the global count. Import names and public-symbol keys are `string_view`s into
the owned string table. Keep that storage alive and stable while the views exist;
copying the unit does not rebase them into the copied string table.

`unit::parse(istream&)` reads scalar fields and operands into ordinary values.
The path overload requires `.bc`, opens a binary stream, and delegates to it.

## Parsing and validation

The parser requires a little-endian host and reads the header, string table,
imports, public symbols, and instruction stream in that order. Stream failures
throw; format checks use always-on `LAMAVM_EXPECT` checks. The parser checks:

- Magic `LaMa`, version 1, a nonempty string table, and metadata count limits.
- Metadata size and each read against the 1 GiB `max_packed_unit_size` limit.
- A terminating zero at the end of the string table and in-range name offsets.
- Nonnegative public-symbol offsets and function/global flags of 0 or 1.
- Known opcode encodings, nonnegative closure capture counts, and capture kinds.
- A final EOF instruction with no trailing bytes.

All recognized opcode forms have interpreter handlers. This is not a full
semantic validator: stack effects, argument counts, string operand
offsets, and global indices are not all validated here. Public global offsets
are not checked against the declaring unit's global count.

## Target coordinates

While reading instructions, `word_by_bc` maps serialized opcode byte offsets to
decoded word indices; operand bytes are not valid instruction boundaries.
A second pass rewrites JMP, CJMP_Z, CJMP_NZ, and nonnegative CALL/CLOSURE targets
through that map. Public function offsets use the same conversion, including
the instruction-boundary check. EOF is a valid mapped boundary.

Global indices, string offsets, argument counts, and closure capture
designations retain their meaning. Negative CALL/CLOSURE operands remain
external references for runtime resolution; they are not local code offsets.

`unit::size` counts logical payload bytes in the four owning containers. It
excludes container objects, spare capacity, allocator overhead, and runtime
global storage. [Dependency loading](units.md) applies a separate aggregate
budget to that value.
