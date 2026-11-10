# CLI arguments

Use cxxopts for the `lamavm` command line. Fetch release 3.3.1 at commit
`44380e5a44706ab7347f400698c703eb2a196202` through CMake and link its
interface target privately to the executable. Follow the existing FetchContent
conventions and mark dependency headers as system headers.

Keep the parser in `app/args.hpp` and `app/args.cpp`. Define `lamavm::args`
with a `ds` boolean and a static `parse(int argc, char **argv)` method returning
`std::expected<args, int>`. Successful parsing returns the arguments. Help and
invalid input print their output inside `parse` and return an unexpected exit
status for `main` to propagate. Keep cxxopts out of `main.cpp` and the header.

Enable bytecode disassembly from standard input only with `--ds`.
Without that flag, return success without reading standard input or printing
disassembly. Preserve the function try block in `main`. Pass `argc` and `argv`
directly to cxxopts. Reject `-ds`, `-d` and `-s`; do not provide legacy aliases.

Provide generated help through `-h` and `--help`, printed to standard output
with exit status 0 without reading bytecode. Reject unknown options and
unexpected positional arguments with a diagnostic and status 2. Keep bytecode
errors at status 1. Other parser-supported boolean flag syntax is allowed.

Remove the retired static analyzer's script, framework model, Nix dependency,
test-script invocation, generated reports and documentation references. Retain
formatting and Clang-Tidy in the test script.

Verify formatting and analysis for the changed source, help and invalid
arguments, silent no-argument behavior and bytecode output with `--ds`. The
golden-test VM wrapper must use `--ds`; the reference compiler keeps its own
`-ds` option. Run the bytecode golden comparisons.
