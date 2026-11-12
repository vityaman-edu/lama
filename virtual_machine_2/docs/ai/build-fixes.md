# Build fixes

- Make `bc/unit.hpp` self-contained by directly including the standard headers
  for every standard-library type used in its declarations.
- Define the existing `fatalf(std::string_view, Ts...)` template so callers can
  link. Format its arguments, print the diagnostic followed by a newline to
  stderr regardless of verbosity, flush stderr, and terminate immediately with
  `EXIT_FAILURE`.
- Preserve the current bytecode data structures and entry point.

Validation: build release and sanitizer targets using the existing pinned
dependency checkout, then verify the stub prints `not implemented yet` and
exits with failure. Check formatting and static analysis of the logging code.

## macOS build environment

On macOS, use system Clang and the system SDK. `scripts/lamavm2.sh` must
unset `DEVELOPER_DIR` and `SDKROOT` before both configuration and compilation.
Clearing them only in the child `configure.sh` process leaves the build using
the inherited Nix toolchain with the configured Apple SDK. Preserve the
standalone configuration script's existing environment handling and leave
other platforms unchanged.

Discover the macOS C, C++, and assembly compilers through system `xcrun`;
do not hardcode compiler paths. Linux must retain the workflow's `CC` and
`CXX` environment settings and Nix compiler selection. Keep the existing
GitHub workflow and Nix packages unless validation identifies a required fix.

Validate shell syntax and rebuild release and sanitizer targets with inherited
Nix SDK variables. Verify that the Linux script branch preserves the CI
environment and does not add macOS compiler or SDK options. Use a writable
`CCACHE_DIR` when running in the sandbox.
