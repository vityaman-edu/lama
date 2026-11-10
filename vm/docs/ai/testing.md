# Testing

## Entry points

Run inside `nix develop` to use the project's compiler and analysis tools.
Build before running checks that need executables:

```sh
./vm/scripts/build.sh release
./vm/scripts/test.sh release
```

Both scripts accept `release` (default), `asan`, `ubsan`, `msan` and `fuzz`.
Outputs live in `vm/build/<mode>/`. The build script builds LaMaC, `lamavm`
and `lamavm_tests`; the test script runs Python lint/format checks, C++
formatting and Clang-Tidy, Dune regressions, golden comparisons, unit
tests and fuzz tests. CI uses these same entry points.

Focused checks:

```sh
./vm/scripts/test-python.sh
./vm/scripts/clang-format.sh --dry-run --Werror
./vm/scripts/clang-tidy.sh release
./vm/scripts/googletest.sh release
./vm/scripts/fuzztest.sh release
python3 vm/test/golden/main.py
```

The golden runner defaults to the release executable; use `--lamavm` for
another mode. See [golden tests](golden-tests.md) for prerequisites
and comparison semantics.

## Bytecode coverage

[Unit tests](../../test/unit/bc/input.cpp) cover signed operands containing
`0xff`, EOF, closure alignment, truncation, invalid opcodes/captures, diagnostic
offsets, file metadata and raw-code delivery, decoder-failure propagation, exact
dump text, aligned tab columns, escaping and external-reference overflow.

The [fuzz properties](../../test/fuzz/bc/input.cpp) independently pass arbitrary
byte strings to `bc::read_file` and `bc::read_code` with no-op listeners. Parsing
may return an error but must not throw or crash. These properties do not exercise
dump-listener validation.

Tests use the helpers in [cmake/Test.cmake](../../cmake/Test.cmake), with CTest
labels `unit` and `fuzz`. Ordinary modes run FuzzTest in unit-test mode.
Coverage-guided `fuzz` builds require Clang on Linux; duration comes from
`LAMAVM_FUZZ_DURATION` (default `10s`).

## Python checks

[scripts/test-python.sh](../../scripts/test-python.sh) checks `scripts/` and
`test/` with Ruff, independently of the caller's working directory. It checks
lint and formatting without modifying files or creating a Ruff cache.
[ruff.toml](../../ruff.toml) owns the rule set and narrow per-file exceptions:
Python 3.10, 100 columns, double quotes, spaces and LF endings.

Fix violations instead of adding inline suppressions; keep unsafe automatic
fixes disabled. Golden-runner imports stay at the top level after the early
`sys.dont_write_bytecode` assignment. Its runtime uses only the Python standard
library.
