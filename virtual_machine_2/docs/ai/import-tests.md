# Import fixtures

Applied scope: add fixtures in `test/data/import` and prepare their dependency
closure in the test compiler wrapper. Do not change the interpreter.

Follow `lama-spec.pdf`, sections 2.2 and 4: each unit has a nonempty body,
imports name compilation units, and dependencies initialize once in import order.

The fixtures are `Leaf`, `Left` and `Right` importing `Leaf`, `Diamond`
importing `Left` then `Right`, and `Reverse` importing `Right` then `Left`.
`Cycle` imports itself and must be rejected before execution.

Keep `.lama` source fixtures and matching `.i` interfaces in the repository.
The bytecode compiler consumes these interfaces but does not generate them.
Use only integer constants, arithmetic, local globals, and internal function calls
supported by `source/interpreter/naive/loop.cpp`. Do not call imported functions,
reference imported globals, or use runtime functions such as `write` or `assert`.
The programs are silent; matching stdout does not establish initialization order.

Compiler-wrapper behavior:

- Supply import-only interfaces for these fixtures, which export no symbols.
- After compiling a source, read its sibling `.i` interface. For each `I,name;`
  declaration with a sibling `name.lama`, compile that source in the same output
  directory. Leave standard-library imports without sibling sources alone.
- Cache each successful compilation before visiting dependencies. This compiles
  shared dependencies once and permits cyclic bytecode graphs to reach the VM's
  cycle detector. Do not reject source cycles in the compiler wrapper.
- Preserve compiler failures and the existing disassembly behavior.
- Validate the import directory and existing examples with 20 runs, and run
  Python style checks. The silent source interpreter does not initialize imported
  units, so it cannot serve as an initialization-order oracle.

Warning policy:

- If both VMs consistently agree on exit code and stdout, their exit code is
  positive, and every `lamai` run exits 0, mark `lamai` WARN instead of FAIL.
- The case is WARN when there are warnings and no implementation failures.
  Count WARN separately in the summary; warnings do not cause a nonzero test
  command exit status. Display warnings in yellow and retain their diagnostics.
- Do not downgrade VM disagreements, timeouts, signal termination, missing
  measurements, or an interpreter that fails in any run. Continue excluding
  stderr from correctness comparisons.

Validation on 2026-11-14, after rebuilding VM2 with the user's removal of the
single-unit restriction:

- `./scripts/test.sh --filter='import/*' --show-stderr`:
  `5 GOOD, 1 WARN, 0 FAIL, 0 SKIP`, with 20 runs per implementation.
  All six sources compiled, and all five acyclic entrypoints passed.
- Both VMs rejected `Cycle` with exit code 1. VM2 reported
  `cyclic import on: Cycle`; the reference VM reported
  `Circular dependency: 'Cycle' -> 'Cycle'`.
  The framework marks `lamai` and the case WARN because `lamai` ignores imported
  unit initialization and exits 0. Both VM results are marked GOOD.
- `./scripts/test.sh --filter='examples/*'`: `9 GOOD, 0 WARN, 0 FAIL, 0 SKIP`.
- `./scripts/check-py.sh`: passed.
- Temporary checks of 12 result combinations and the WARN/FAIL command exit
  statuses passed, covering mismatches, unstable runs, missing results, crashes,
  timeouts, and stderr-only differences.
