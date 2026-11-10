# C++ cleanup specification

- Extract byte escaping into `quote(unsigned char) -> std::string` in an anonymous
  namespace in `lib/support/string.cpp`, preserving every existing escape.
- Replace bytecode reader scope entry/exit pairs with
  `scoped(std::string_view name, auto f)`, returning the callback result and
  restoring the previous diagnostic scope after the callback returns.
- Reserve initial string-table capacity up to the read buffer size. The declared
  size is untrusted, so malformed input must not trigger a large allocation
  before its bytes are consumed.
- Move the general `lamavm::exception` module from `lib/bc/core` to `lib/core`.
  Update includes, CMake recursion and dependencies to use `liblamavm_core`.
- Preserve parsing results, diagnostic scope names and disassembly output.
- Validate formatting, the existing bytecode unit tests and fuzz tests, and the
  project analysis checks. Run Cppcheck at the exhaustive check level so scoped
  callbacks do not limit branch analysis or fail the check on that information.
