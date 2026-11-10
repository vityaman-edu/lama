# Golden tests

## Runner and adapters

[test/golden/main.py](../../test/golden/main.py) compares LaMaVM
disassembly against a freshly generated LaMaC stack-machine dump for each
`.lama` source. It uses Python's standard-library `unittest`, independently of
CMake/CTest. It does not execute corpus programs or read `.input` or Cram files.

```sh
python3 vm/test/golden/main.py \
  --lamac /absolute/path/to/vm/scripts/lamac.sh \
  --lamavm /absolute/path/to/vm/build/release/lamavm \
  --timeout 30
```

All options are optional. Executable defaults are resolved relative to the
runner, so it works from any directory. Timeout is positive finite seconds per
subprocess. Build prerequisites are handled by `scripts/build.sh` and the Dune
step in `scripts/test.sh`; the runner does not build tools.

- [LaMaC](../../test/golden/lamac.py) takes source text, writes
  `program.lama` in a temporary working directory and invokes
  `lamac.sh -b -ds program.lama`. It returns binary `.bc` and text `.sm` contents;
  the reference is the `.sm` file, not stdout.
- [scripts/lamac.sh](../../scripts/lamac.sh) invokes the built driver with runtime
  and stdlib include paths. Calling `Driver.exe` directly needs those paths too.
- [LaMaVM](../../test/golden/lamavm.py) passes bytecode to `lamavm --disassembly=legacy`
  on binary stdin and returns decoded stdout. Nonzero exit status fails even
  when stdout contains a partial dump.
- Adapters resolve executable paths before changing working directories and
  use argument vectors without a shell, captured output and timeouts. Temporary
  outputs are read before cleanup; generated goldens are not committed.
- `LaMaC.interpret(source, stdin)` is a thin `-i` adapter;
  `LaMaVM.interpret` raises `NotImplementedError`. Execution comparisons are
  outside the current suite.

## Comparison contract

[test_printbc.py](../../test/golden/test_printbc.py) parses and normalizes
each dump independently, then compares canonical text exactly. Expected values
come only from the compiler dump. Keep normalization here rather than changing
the production printer to satisfy tests. The owning source for serialization
and dump syntax is [src/SM.ml](../../../src/SM.ml).

1. Resolve compiler `LABEL`, `FLABEL` and `SLABEL` definitions to serialized
   code offsets, then omit declarations on both sides. Instruction sizes include
   variable-sized closures and the shorter dedicated `.array` call encoding.
   Preserve all branch, call and closure destinations. A defined internal label
   takes precedence over an external declaration.
2. Track `EXTERN` declarations before omitting them. Preserve external symbol
   names; map internal globals to indices in first serialized-use order,
   including globals in closure captures. Preserve designation kinds and local,
   argument and captured-variable indices.
3. Reduce `BEGIN` to argument, local and capture counts: source names, argument
   names, scopes and capture identities are absent from bytecode. Preserve
   `CLOSURE` capture identities, kinds and order.
4. Omit only the tail-call boolean from `CALL` and `CALLC`. Preserve arity,
   including `.array` arity, and `FAIL`'s keep-value boolean and source position.
5. Decode OCaml dump quoting, including decimal byte escapes. On the compiler
   side additionally apply serializer literal unescaping to `STRING`, `SEXP`
   and `TAG`, which pass through `add_strings`. Do not apply it to symbol names.
6. Compare imports and public names/flags in table order, followed by
   instructions in original order. EOF is not printed. Reject unknown or
   malformed dump syntax rather than dropping unmatched text; preserve string
   contents when normalizing text layout.

The dumps do not expose all header fields, raw string-table layout or
public-symbol addresses. This suite compares exposed information, not the
complete binary representation. Mismatches report the corpus-relative filename,
a unified `lamac`/`lamavm` diff and both raw dumps.

## Corpus and expected failures

The runner discovers and sorts only `.lama` files recursively under
`test/golden/corpus/`, following directory symlinks, with one named case per
source. The [regression symlink](../../test/golden/corpus/regression) points to
repository-root `regression/`; local fixtures such as `examples/42.lama` use
the same comparison. Case names are relative to `corpus/`, including the
`regression/` prefix for legacy cases. Missing executables or a missing or
empty corpus directory are setup errors. Do not hardcode a source count or
silently omit cases.

Discovery ignores `.input` and `.t` files. Input handling and `.t` parsing
(including legacy files starting with `$ --` and the custom format) are future
work, outside this runner's current contract.

`KNOWN_FAILURES` in `test_printbc.py` lists `regression/test054.lama` and
`regression/test110.lama`:
LaMaC rejects their indirect assignments with exit status 2 and
`Indirect assignment is not supported yet`. Still compile these sources and
skip only when both status and diagnostic match. Different failures, timeouts
and unexpected success fail and require investigation of the expectation.

The runner continues after failures, prints per-case `GOOD`, `FAIL` or `SKIP`
and a summary, and returns nonzero on setup/test failure. Documented skips are
compatible with success. Keep VM-specific fixtures under `test/golden/corpus/`.
