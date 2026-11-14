# Bytecode dependency loading

Status: implemented and checked on 2026-11-14.

Implement the traversal from [preclosure](https://github.com/vityaman-edu/lama/commit/15880d4a16cdd36e7eeb32fd91172e621e619057)
for the current `bc::units` representation.

- Identify units by `path.stem().string()` and load each name once.
- Validate that imports are acyclic with
  `std::unordered_set<std::string_view> started` in the loader. After checking
  already loaded units, reject a name already present in `started`, identifying
  the cyclic import in the diagnostic.
- Insert a view of the local unit name before loading its dependencies. Remove
  it after the dependencies finish and before moving the name into `by_name`,
  so every view in `started` refers to a live, unchanged string.
- Parse a unit, account for its size once, then visit its imports in order.
- Resolve each import as `path.parent_path() / (import_name + ".bc")`.
- Append the importing unit after all its dependencies.
- Return every loaded unit in `by_name` and its map iterator in
  `initialization_order`, with dependencies preceding importers.
- Use `std::map` for `by_name`, so insertions preserve existing iterators.
  Append each inserted unit's iterator directly to `initialization_order`.
  No separate ordering container is needed in the loader.
- Preserve parser failures for missing or invalid bytecode.

`unit::size` returns logical payload bytes: string-table elements, text words,
import views, and public-symbol map entries, each multiplied by its element
size. It excludes container objects, spare capacity, allocator overhead, and
globals, whose storage has not been allocated in `unit`.

The accumulated payload must not exceed the existing 1 GiB bound,
`max_packed_unit_size`. Check the remaining budget before adding a unit's size.

Validation uses synthetic bytecode for a leaf, a dependency chain, shared and
repeated imports, a missing dependency, and a larger dependency graph. Reject
both self-imports and cycles involving multiple units.
Also check payload accounting and the exact total-size boundary.

The release build, formatting, and temporary fixture checks passed, including
an 84-unit traversal, nested import paths, duplicate accounting, exact-limit
acceptance, and one-byte-over-limit rejection.
