# Build repair specification

Apply the existing `bc::unit` interface to its callers:

- Parse the input with `bc::unit::parse` in `main`.
- Keep the owning `bc::unit` alive throughout the interpreter loop.
  Obtain string-table and code spans through its public accessors and remove
  the obsolete separate byte-buffer owner.
- Obtain the global count from `unit.header().globals_count` and use a matching
  signed loop index.
- Mark the `BEGIN` argument count and interpreter load flag as potentially
  unused because their assertions are compiled out in release builds;
  continue consuming the operand.

Preserve opcode behavior, assertions, and compiler warnings. Validate with
release and sanitizer builds and CLI missing-argument smoke checks.
