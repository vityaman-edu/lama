# Legacy golden test experiment — 2026-11-09

The implementation was discarded at the user's request: it was too bloated.
Keep these findings for a separate, smaller implementation later. No production
bug fix was identified or retained. The existing `lamac_ds.hpp` / `.cpp` names
should stay unchanged.

## Results

The final run rebuilt the driver and executed:

```sh
ctest --test-dir vm/build/release --output-on-failure --label-regex '^golden$' --parallel 6
```

- 88 tests ran: 79 generated corpus cases and nine helper tests.
- 86 passed; two failed. All 77 successfully compiled corpus sources matched
  the normalized compiler disassembly, including `test803`.
- `test054.lama` and `test110.lama` fail inside lamac before producing bytecode.
  Both report `Indirect assignment is not supported yet`. The former selects
  between references to `x` and `y`; the latter between `x[0]` and `y[0]`.
  These are compiler limitations, not decoder mismatches. They need an explicit
  policy in a future suite; silently dropping these sources would hide coverage.
- Eight existing bytecode unit tests also passed in an earlier combined run.

## Useful repository facts

- The relative link from `vm/test/golden/data-legacy` to `regression/` is
  `../../../regression`. The original node was an empty directory.
- `regression/` contains 79 `.lama` files. `regression/vm/` contains Cram tests,
  not more sources. Discover filenames rather than assuming contiguous numbers.
- `vm/scripts/lamac.sh` already wraps the built compiler with runtime and include
  paths. It forwards arguments and preserves the caller's working directory.
- `lamac -b -ds /absolute/source.lama` writes `source.bc` and `source.sm` in its
  working directory. The golden is the `.sm` file, not stdout.
- An isolated working directory per case allows parallel compilation and avoids
  writing generated files into the source corpus. Paths with spaces worked.
- `TEST_P` with a generated, sorted file manifest and `gtest_discover_tests`
  registered each source separately. CMake `GLOB CONFIGURE_DEPENDS` refreshed
  discovery after adding and removing a temporary source and rebuilding.
  Listing tests did not invoke lamac. `BUILD_TESTING=OFF` also configured with
  the corpus absent.

## Comparison details worth preserving

Raw dump equality is impossible. Compiler `.sm` includes labels, external
declarations, source global names, BEGIN argument names and scopes, original
BEGIN capture designations, and tail-call flags that bytecode does not retain.

The successful comparisons resolved labels to code offsets, assigned internal
global indices in serialization order, preserved external symbol names, and
normalized only unavailable metadata. BEGIN capture identities disappear, but
their count remains; CLOSURE capture kinds and indices must still compare.
CALL/CALLC tail-call flags disappear, while FAIL's keep-value flag is encoded.
The `.array` call has a shorter, dedicated encoding and affects label offsets.
Compiler dump quoting and the compiler's additional literal unescaping are two
separate steps. Expected values must come from `.sm`, independently of `.bc`.

## Lessons for the next attempt

The draft grew to roughly 800 lines of C++ across a driver, subprocess helper,
dump parser/normalizer and helper tests. Do not carry that infrastructure forward
by default. Start with a small case and the existing wrapper, establish the
minimal comparison contract, and review complexity before expanding coverage.

Clang-Tidy must use the Nix toolchain's version 23; the ordinary PATH exposed
version 21 and produced incompatible diagnostics. Formatting and Clang-Tidy
passed after adjustments. The full project check stopped at Cppcheck's two
`useStlAlgorithm` findings in normalization loops, before reaching the Dune
regressions. The final golden run above was executed separately.
