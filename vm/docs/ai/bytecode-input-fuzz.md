# Bytecode input fuzz test

- Add `test/fuzz/bc/input.hpp` with a FuzzTest property supplying arbitrary byte strings to `bc::read_file` through an input stream and a no-op `base_listener`.
- Require parsing to return without throwing or crashing; malformed input may return an error.
- Register the property in a bytecode fuzz target using the existing CMake helpers.
- Fix Clang-Tidy diagnostics without disabling checks.
