# Legacy golden testing with LaMaC

Status: initial bytecode comparison implemented; program execution is deferred.
This replaces the earlier C++/GoogleTest proposal. Paths are relative to `vm/`
unless stated otherwise.

## Scope and layout

Compare the built `lamavm` bytecode dump with LaMaC's stack-machine dump for
each legacy `.lama` source. Generate the reference on every run; do not commit
generated goldens. Use Python 3 and its standard library only. Add `python3`
to the repository's `flake.nix`. No CMake, CTest or GoogleTest integration.

```text
scripts/
  test.sh
test/golden-legacy/
  data/
    regression -> ../../../../regression
  lamac.py
  lamavm.py
  test_printbc.py
  main.py
```

The symlink points to repository-root `regression/`. Keep `test/golden/data/`
for VM-specific fixtures. Keep normalization in `test_printbc.py` initially;
no general process or testing framework is needed. Leave the production
`lamac_ds` printer's names and normalization-independent behavior unchanged.

Execution and stdout comparison against supplied stdin are future work. The
initial suite must not execute corpus programs or read `.input` or Cram files.

## Executable adapters

Both classes receive an executable path and a subprocess timeout at construction.
Resolve paths before entering temporary directories. Use `subprocess.run` with
argument vectors, without a shell. Capture stdout and stderr, check exit status,
and enforce a per-invocation timeout (default: 30 seconds). Errors retain the
command, status or timeout, stdout and stderr.

`source: str` means source text, not a path. Each LaMaC call writes UTF-8 text
to `program.lama` in its own `TemporaryDirectory` and sets the child's working
directory there. Read results before cleanup. Leave the caller's working
directory and corpus untouched. Text uses UTF-8; bytecode remains binary.

### `lamac.py`

```python
class LaMaC:
    def bytecode(self, source: str) -> tuple[bytes, str]: ...
    def interpret(self, source: str, stdin: str) -> str: ...
```

- Default executable: absolute `scripts/lamac.sh`, which invokes repository-root
  `_build/default/src/Driver.exe` with runtime/include paths. Do not invoke the
  devshell's shell function. An override must support the same arguments;
  invoking `Driver.exe` directly also needs the wrapper's runtime/include flags.
- `bytecode` runs `lamac.sh -b -ds program.lama`, requires `program.bc` and
  `program.sm` after successful compilation, and returns their contents in that
  order. The reference dump comes from `.sm`, not stdout.
- `interpret` runs `lamac.sh -i program.lama`, sends UTF-8 encoded `stdin`, and
  returns decoded stdout without trimming. Implement this thin adapter now;
  execution comparisons and corpus execution remain outside this phase.

### `lamavm.py`

```python
class LaMaVM:
    def printbc(self, bc: bytes) -> str: ...
    def interpret(self, bc: bytes, stdin: str) -> str: ...
```

- `printbc` runs the selected built `lamavm -ds`, passes `bc` to binary stdin,
  and returns decoded stdout. Decoder failure fails even with partial stdout.
- The current executable dumps stdin without parsing arguments. Add explicit
  `-ds` support, preserve no-argument dump mode, and reject unsupported arguments
  with nonzero status. No filename argument is needed in this phase.
- `interpret` raises `NotImplementedError`; implementing VM execution is deferred.

## `test_printbc.py`: comparison contract

For each source, read its text, call `LaMaC.bytecode`, then pass the returned
bytes to `LaMaVM.printbc`. Compare `normalize_lamac(reference)` and
`normalize_lamavm(actual)` for exact equality. Both functions return canonical
text and are pure functions of their respective dumps. Never derive expected
values from `.bc` or the actual dump.

Bytecode omits compiler metadata. Parse enough dump syntax to respect quoted
strings, escapes, lists, tuples and nested scope records. Reject unknown or
malformed forms rather than dropping unmatched text. The owning source for
compiler dump syntax and serialization is repository-root `src/SM.ml`.

1. Resolve compiler `LABEL`, `FLABEL` and `SLABEL` definitions to byte offsets
   from the start of the serialized instruction stream. Compute instruction
   sizes from `src/SM.ml`, including variable-sized closures and the dedicated
   `.array` call encoding. Convert internal branch/call/closure targets to these
   offsets. Omit label declarations on both sides, including synthetic VM
   function-entry labels. Preserve every referenced destination.
2. Track `EXTERN` declarations before omitting them. Preserve external names;
   map internal global names to indices in first serialized-use order, including
   globals in closure captures. Preserve designation kinds and local, argument
   and captured-variable indices. A defined internal function label takes
   precedence over an external declaration, as in the compiler fixup logic.
3. Canonicalize `BEGIN` to argument, local and capture counts. Its source name,
   argument names, scope tree and capture identities are not serialized.
   Preserve capture identities, kinds and order in `CLOSURE`.
4. Omit only the tail-call boolean in `CALL` and `CALLC`. Preserve arity,
   including `.array` arity. Preserve the `FAIL` boolean and source position:
   the two failure variants have separate opcodes.
5. Decode OCaml dump quoting, including decimal byte escapes. On the compiler
   side additionally apply the serializer's literal unescaping only to fields
   passed through `add_strings`: `STRING`, `SEXP` and `TAG`. Compare resulting
   bytes with common deterministic escaping. Do not unescape symbol names or
   arbitrary text this way.
6. Compare imports and public names/flags in serialized table order, followed
   by instructions in original order. Omit no other instruction or operand.
   EOF is a bytecode terminator, not a printed instruction. Normalize CRLF and
   the final newline explicitly, without altering string contents.

This compares information exposed by the dumps. Header fields, raw string-table
layout and public-symbol addresses are not all printed by the current listener;
the suite does not compare the complete binary file representation.

On mismatch report the corpus-relative filename and a unified diff labeled
`lamac` and `lamavm`. Include raw dumps in the diagnostic to help distinguish
normalization defects. Temporary files stay ephemeral; no artifact archive or
persistent cache is required initially.

## Discovery and known compiler failures

Discover and sort `data/regression/**/*.lama` at runtime. Register one named
standard-library `unittest` case per source, identified by its relative path.
Continue after case failures and print unittest results and summary. Missing
executables, a broken symlink or an empty corpus are errors, not skips. Do not
hardcode a source count; the current corpus has 79.

Two sources currently fail in LaMaC with exit status 2 and the diagnostic
`Indirect assignment is not supported yet`: `test054.lama` and `test110.lama`.
Keep a small explicit filename-to-expected-status-and-diagnostic mapping. Still
invoke the compiler for both. Report a matching failure as a named skip with its
reason. A different failure, timeout, or unexpected compiler success fails the
case and requires investigation of the expectation. Other compilation failures
fail normally. Do not exclude `test803.lama`.

## `main.py` and integration

`main.py` parses arguments, constructs adapters, discovers the corpus and runs
the suite from `test_printbc.py`. It works from any current directory, with
script-relative absolute defaults:

```sh
python3 vm/test/golden-legacy/main.py \
  --lamac /absolute/path/to/vm/scripts/lamac.sh \
  --lamavm /absolute/path/to/vm/build/release/lamavm
```

Both arguments are optional: defaults select the repository wrapper and
`build/release/lamavm`. Accept `--timeout` as positive seconds. Return 0 when all
comparisons pass with only documented compiler skips; return nonzero on any
setup or test failure.

After the existing Dune regressions, `scripts/test.sh` invokes:

```sh
python3 "$vm_dir/test/golden-legacy/main.py" \
  --lamavm "$vm_dir/build/$mode/lamavm"
```

Preserve build-mode selection and error propagation. Build prerequisites belong
to `scripts/build.sh` and the existing Dune step; the Python runner does not
build tools. Verify compiler interface files needed by the corpus are available
through the wrapper. Existing VM CI enters Nix and invokes `scripts/test.sh`
for each mode, so no new CI job or Python package installation is needed.

## Acceptance

- Nix provides `python3`; the runner needs no third-party Python dependencies.
- The runner works from another directory and with paths containing spaces.
- All current sources are reported: 77 comparisons and the two narrowly checked
  compiler skips, unless the compiler or corpus changes.
- Focused normalization checks show that changing a constant, branch destination,
  string byte or closure capture fails comparison, while compiler-only metadata
  differences normalize equally.
- Unknown dump syntax, missing output files, invalid bytecode, nonzero child
  status and timeouts produce useful failures.
- `scripts/test.sh` selects the correct executable for its build mode. Existing
  unit/fuzz checks remain runnable. No generated files appear in the corpus.
- Execution comparisons, parallel scheduling, persistent artifacts and more
  corpus families remain future work.
