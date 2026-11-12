# Verbose logging

- Define `lamavm::is_verbose` as a compile-time boolean: true when `VERBOSE`
  is defined (including `VERBOSE=0`), false otherwise.
- Implement `verbosef` with its existing `std::string_view` format and variadic
  arguments. When verbosity is disabled, perform no formatting or output.
- When verbosity is enabled, format the message with standard C++ formatting
  and write it to stderr, without adding a newline or prefix.
- Leave `fatalf` unchanged; its behavior is outside these TODOs.

Validation: compile and run a small caller with `VERBOSE` undefined, defined,
and defined as zero. Check formatted stderr output in enabled builds and silence
with an invalid format string in the disabled build.
