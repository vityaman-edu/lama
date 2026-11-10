# Bytecode and CLI

## Decoder

The compiler's [src/SM.ml](../../../src/SM.ml) owns serialization and dump syntax.
The VM's [layout.hpp](../../lib/bc/layout.hpp) defines packed version-1 wire
structures and `opcode`. Headers contain declarations; implementations live in
`.cpp` translation units, except the templated `read_op` wrapper and its decoder in
[code_reader.detail.hpp](../../lib/bc/code_reader.detail.hpp).

- `read_file(stream, file_listener&)` parses the header, string table, imports
  and public symbols, then passes the remaining code bytes to a single
  `std::span<const std::byte>` callback. This callback returns
  `std::expected<void, std::vector<std::string>>` so a decoder failure propagates
  to the file caller. A file listener decides whether to validate, decode or retain
  the code.
- `read_ops(span, code_listener&)` decodes instructions independently of file
  metadata. It checks bounds and passes packed instruction views and closure-capture
  spans directly from the input buffer, without copying or allocating. Mutable byte
  spans can also be passed to this read-only interface.
  The entry points are declared in [file_reader.hpp](../../lib/bc/file_reader.hpp)
  and [code_reader.hpp](../../lib/bc/code_reader.hpp), respectively.
- `file_listener` and `code_listener` have separate headers and translation
  units. The former handles metadata and the code span; the latter has one
  typed callback per instruction. Every callback is pure virtual. There is no
  diagnostic callback. Implementations provide all callbacks explicitly.
  `code_listener_nop` implements every code callback as a no-op and supports
  deriving listeners that override selected callbacks. `file_listener_nop` ignores
  every file callback and returns success for code. `file_listener_seq` owns two
  file listeners through unique pointers, forwards every callback to both in order,
  and combines returned code diagnostics in that order. Pure virtual destructors
  are defined out of line, while construction and copy operations are implicit. The
  `cppcoreguidelines-special-member-functions` check is disabled in `.clang-tidy`
  to allow this interface design without explicit special-member boilerplate.
- `read_op<Listener, bool unsafe = false>(code, listener)` is templated on the
  concrete listener type and compile-time validation mode, without
  type erasure or a required `code_listener` base. It decodes exactly one
  instruction from the span prefix, including its closure captures, and returns `std::expected<void, std::string>`.
  The listener advances its own offset. No offset argument or following EOF is
  required. With validation enabled, empty spans fail. Diagnostic offsets are
  relative to the span.
  `read_ops` retains its `code_listener&` interface and shares the same decoder
  to read through EOF. It returns
  `std::expected<void, std::vector<std::string>>`. Decoding stops at the first
  invalid instruction.
- `read_op_unsafe(code, listener)` calls `read_op<Listener, true>`. Unsafe mode
  removes decoder checks at compile time: code size, bounds, capture count/kinds
  and unknown opcodes. It requires a complete, valid instruction prefix; invalid
  input has undefined behavior. It returns the same expected status as `read_op`.
  `read_ops` keeps validation enabled.
- File diagnostics use file offsets; code diagnostics use code-section offsets.
  Error strings include `offset N: scope: message`. `read_file` collects
  recoverable metadata errors, stops on framing failures, and returns all collected
  diagnostics in its error vector. It propagates code-span callback errors unchanged.
  Invalid metadata returns an unexpected result; listeners validate instructions.
  Listeners can throw, including printers rejecting invalid string references.
- The bytecode module requires a little-endian host, enforced by a static
  assertion. Packed wire views require byte alignment, also checked statically.
  Operands are little-endian signed 32-bit values. A single `0xff` at an
  instruction boundary emits EOF and ends code parsing. Operand bytes cannot
  terminate parsing. Missing EOF is a code error, not a file framing error.
- `CLOSURE` contains a target and capture count, followed by count pairs of an
  8-bit designation kind and signed 32-bit index. Negative counts and kinds
  outside 0–3 are rejected before the complete instruction callback.
- The string-table view, code span, instruction references and closure-capture
  spans are borrowed. Instruction and capture views point into the caller's code
  buffer; keep that storage alive while using them. File callbacks borrow data
  owned by the file reader and must copy data retained after parsing returns.
- Read stream data in bounded chunks. Untrusted sizes must not cause large
  allocations before the bytes are consumed. Keep opcode dispatch exhaustive,
  without a default branch, and diagnose unknown opcode values after the switch.

## Disassembly

The `bc/disassembly` module exposes `disassembly::modern` in
[modern.hpp](../../lib/bc/disassembly/modern.hpp) and `disassembly::legacy` in
[legacy.hpp](../../lib/bc/disassembly/legacy.hpp). Both public classes derive
directly from `file_listener` and `code_listener`; all implementations remain
in their corresponding `.cpp` files. Construct a listener with an output stream,
then pass it to `read_file`, or forward retained metadata
and code callbacks before executing the same code bytes. Listeners also work
with retained code: pass its span to the listener after supplying the metadata.
Creating a printer does not consume input. The stream must outlive the listener.
The CLI selects a `std::unique_ptr<file_listener>` in the disassembly-kind
switch, using `file_listener_nop` for `none`, and composes it with
`interpreter::boot` through `file_listener_seq`. It reads stdin and prints returned
diagnostics to stderr in one common path.

The printer writes one line per instruction, including EOF:
`<decimal code offset>:\t<uppercase hex bytes separated by spaces>\t<symbolic>`.
The three columns are separated by literal tab characters. In the file listener's
code-span callback, `read_ops` collects instruction rows into a vector. After
decoding succeeds, the listener computes column widths and formats those rows.
The code-span callback validates while collecting listing rows.
There is no code-start callback. Padding before tabs keeps
columns aligned even beyond a single tab stop.
A full-file dump starts with `header`, `string_table`, `imports` and
`public_symbols` sections, including empty tables. Header fields include magic,
version, string-table byte size and all counts. String entries show their decimal
byte offset and escaped value. Imports use `index:\t"name" (name_offset)`.
Public symbols use `index:\tkind\t"name" (name_offset) at code_offset`, where
kind is `function` or `global`. All section contents are indented two spaces;
table columns use aligned tabs.
A `code` section follows, with offsets
starting at zero. Metadata callbacks write directly to the stream. Passing a
code span to the modern listener collects and formats its code listing, including
when printing retained code before execution.
Bytes retain their little-endian file order;
closure captures belong to the same line as their instruction. Mnemonics use
uppercase opcode names (`CONST` for `constant`), with decimal operands separated
by commas. String operands and negative external references resolve to escaped,
quoted names. Closure captures use explicit global/local/arg/captured kinds.

`--disassembly=modern` selects this format; `--disassembly=legacy` selects the
LaMaC-compatible printer below. `--disassembly=none` is the default and validates
stdin without printing output. Invalid kinds are command-line errors (exit status 2).
Error message prose uses lowercase text; external diagnostics and embedded values
retain their original spelling.

`disassembly::legacy` writes LaMaC-style stack-machine text to an ostream.

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
`args::parse(argc, argv) -> std::expected<args, std::string>`; the error string
contains a diagnostic, and `main` prints it to stderr and returns status 2.
Help prints usage and sets `args::is_help`; `main` exits without consuming stdin.
cxxopts stays out of `main.cpp` and the public header. Its dependency pin lives in
[cmake/Cxxopts.cmake](../../cmake/Cxxopts.cmake).

| Invocation | Behavior | Exit status |
| --- | --- | --- |
| `lamavm --disassembly=modern < program.bc` | Disassemble stdin; diagnostics go to stderr | 0 on success, 1 on bytecode failure |
| `lamavm` | Validate stdin without printing output | 0 on success, 1 on bytecode failure |
| `lamavm -h` / `lamavm --help` | Print generated help to stdout without reading bytecode | 0 |
| Unknown option or positional argument | Print a diagnostic | 2 |

LaMaC uses `-ds`; LaMaVM uses `--disassembly=legacy` for compatible output.
VM program execution is not implemented.

## Interpreter scaffolding

`interpreter::boot` is a file listener whose metadata callbacks do nothing. Its
code callback calls `read_ops` with `code_listener_nop` and returns its validation
result. `interpreter::loop` is a code listener; every instruction callback,
including EOF and closures, throws `lamavm::exception` reporting an unsupported
instruction. Boot does not execute the loop yet.

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
