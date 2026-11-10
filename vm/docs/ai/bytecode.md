# Bytecode and CLI

## Decoder

The compiler's [src/SM.ml](../../../src/SM.ml) owns serialization and dump syntax.
The VM's [layout.hpp](../../lib/bc/layout.hpp) defines packed version-1 wire
structures and `opcode`; [input.hpp](../../lib/bc/input.hpp) implements
`bc::read_file(stream, listener)`.

- The header, string table, imports and public symbols precede instructions.
  The string table reaches the listener before any entries referencing it.
- Operands are little-endian signed 32-bit values. A single `0xff` at an
  instruction boundary emits EOF and ends parsing; operand bytes do not
  terminate the stream.
- `CLOSURE` contains a target and capture count, followed by count pairs of an
  8-bit designation kind and signed 32-bit index. Negative counts and kinds
  outside 0–3 are rejected. A complete instruction produces one typed callback.
- The string-table view and closure-capture span are borrowed. Copy data needed
  after parsing or after the capture callback, respectively.
- Malformed input returns an unexpected result and reports a file offset and
  diagnostic scope through the listener. Listeners can throw; the dump listener
  uses `lamavm::exception` for invalid string references.
- Untrusted sizes must not cause large allocations before data is consumed. Initial
  string-table capacity is capped at the read buffer size.
- Keep opcode dispatch exhaustive without a default branch. Unnamed opcode
  values reach the unknown-opcode diagnostic after the switch.

## Disassembly

[lamac_ds_listener](../../lib/bc/lamac_ds.cpp) writes LaMaC-style stack-machine
text to an ostream.

Bytecode loses source labels, local/global names, scopes and tail-call flags.
The printer uses code offsets and indices for unavailable identities. `BEGIN`
retains the three list positions for captures, argument names and lexical
scopes; empty names/scopes and positional `Access` captures stand in for
missing metadata. See the [comparison contract](golden-tests.md) for
which differences can be normalized.

String offsets are signed. Decode a negative external reference with
`-(value + 1)` so `INT32_MIN` is handled without overflow. String references
must point inside the table to a NUL-terminated string.

[support::quote](../../lib/support/string.cpp) provides OCaml dump escaping,
including three-digit decimal byte escapes. `std::quoted` and debug formatting
do not implement this format. Explicit VM throws use
[lamavm::exception](../../lib/core/exception.hpp), derived from
`std::runtime_error`.

## Command line

[app/args.cpp](../../app/args.cpp) wraps cxxopts behind
`args::parse(argc, argv) -> std::expected<args, int>`; cxxopts stays out of
`main.cpp` and the public header. Its dependency pin lives in
[cmake/Cxxopts.cmake](../../cmake/Cxxopts.cmake).

| Invocation | Behavior | Exit status |
| --- | --- | --- |
| `lamavm --ds < program.bc` | Disassemble stdin; diagnostics go to stderr | 0 on success, 1 on bytecode failure |
| `lamavm` | Return without reading stdin or printing output | 0 |
| `lamavm -h` / `lamavm --help` | Print generated help to stdout without reading bytecode | 0 |
| Unknown option or positional argument | Print a diagnostic | 2 |

LaMaC uses `-ds`; LaMaVM uses `--ds`. `-ds`, `-d` and `-s` are not VM aliases.
VM program execution is not implemented.

## LaMaC wrapper

[scripts/lamac.sh](../../scripts/lamac.sh) forwards ordinary invocations to
LaMaC. With `--better`, it implies `-b -ds`, writes binary bytecode to stdout
and the stack-machine symbol dump to stderr. Existing `-b` and `-ds` flags
are accepted alongside `--better`.

Compilation runs in a fresh directory under `${TMPDIR:-/tmp}`, removed on exit
including compilation failures. Relative source, include and runtime paths
retain their meaning from the caller's working directory. Compiler diagnostics
go to stderr; failed compilations preserve the compiler's exit status and emit
no bytecode. Generated files stay out of the caller's working directory.

```sh
./vm/scripts/lamac.sh --better program.lama > program.bc 2> program.sm
```
