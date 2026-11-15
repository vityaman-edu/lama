# Dependency loading and symbols

[`bc::units::from`](../../source/bc/units.cpp) loads an entry unit and its
bytecode dependencies. It returns owning units in `by_name`, dependency-first
map iterators in `initialization_order`, and a shared public-symbol index.

## Loading

- A unit's identity is `path.stem().string()`. Already loaded names are reused;
  identity does not distinguish equal stems in different directories.
- Imports are visited in declaration order. Each dependency path is relative
  to its importer's directory, with `.bc` appended to the import name.
- `Std` is skipped by the bytecode loader; the C runtime supplies the currently
  supported standard operations.
- An in-progress-name set detects cycles. Missing or invalid files retain
  parser failures; cycles are rejected before execution.
- Each importing unit is appended after its dependencies. A `std::map` owns
  units so later insertions preserve the saved iterators.

The loader accounts for each unit's logical payload once, using
[`unit::size`](bytecode.md). Before adding it, the loader checks the remaining
aggregate budget against `max_packed_unit_size` (1 GiB). This logical payload
budget differs from the per-file serialized-byte limit and from total memory use.

## Public symbols

The shared index maps each public name to its function/global kind, offset,
and declaring unit's index in `initialization_order`. Function offsets are
already word indices; global offsets remain global indices.

Public names share one namespace across the loaded dependency closure.
Duplicate names are rejected as ambiguous, even when an importing unit does
not call that symbol. This is not a per-import namespace or lazy resolution.

Cold CALL, LD_GLO, and ST_GLO resolve negative operands through this index and
check the symbol kind. A negative operand encodes the symbol-name string offset
as `-offset - 1`; lookup uses the current unit's string table.

## Execution and lifetime

[`interpreter::run`](../../source/interpreter/run.cpp) owns the loaded units
for execution, decodes their handler words, and creates runtime contexts in
initialization order. Each context borrows its unit's string and word buffers
and records the base of its globals in the shared stack.

Globals are allocated before unit bodies run. Bodies then run once in dependency
order. Imported calls select the target unit context; END restores the caller's
context. See [the interpreter](threaded-code.md) and [runtime frames](runtime.md).

The current loader retains separate unit buffers. Combining them into one
linked allocation is a [future design](linking.md).
