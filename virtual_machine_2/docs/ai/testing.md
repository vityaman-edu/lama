# Differential testing

[`scripts/test.sh`](../../scripts/test.sh) runs
[`test/main.py`](../../test/main.py) against the source interpreter `lamai`,
the reference VM `lamavm`, and `lamavm2`. Run commands from `virtual_machine_2`;
build prerequisites are described in [Tooling](tooling.md).

```sh
./scripts/test.sh --filter='examples/*'
./scripts/test.sh san --filter='import/Sum4*' --runs=2 --show-stderr
```

Discovery follows directory symlinks and selects sorted `.lama` files under
`test/data`; `--filter` matches paths relative to that directory. Each runner
receives identical bytes from the sibling `.input` file, or empty input if it
is absent. Compilation and generated files use a temporary directory per case.

## Results and measurements

- Correctness compares exit code and stdout across every run against the first
  reference VM result. Stderr is diagnostic only and is never compared.
- `FLAKY_STDOUT` lists cases compared by exit code alone, including reference
  stability checks and failure diagnostics. It contains `regression/test803.lama`:
  the nested function `g` shadows parameter `g`, so `write(g)` prints a closure
  pointer interpreted as an integer, which varies between processes.
- A matching positive exit code can be GOOD: agreement on failure is distinct
  from successful execution. Timeouts, signal termination, unstable results,
  or VM disagreements fail the case.
- A sibling `.i` interface skips `lamai` without running or benchmarking it.
  Both VMs are still compared normally; a passing case remains GOOD.
- `LAMAI_UNSUPPORTED` also skips `lamai` without running or benchmarking it.
  It contains `regression/test111.lama`, whose `stringcat` builtin is absent
  from `lamai`, and `regression/test803.lama`, whose closure argument to `write`
  is rejected by `lamai`. Both VMs remain subject to comparison.
- For cases where `lamai` runs, consistent agreement between both VMs on a
  positive exit code while every `lamai` run exits zero produces WARN for
  `lamai`. WARN does not make the test command fail.
- Known compiler failures are SKIP only when their expected exit code and
  diagnostic match. Unexpected acceptance requires reviewing that exception.
- Default sampling is 20 runs per implementation; `--runs` requires at least
  two. Reports show absolute wall time, a 95% bootstrap confidence interval,
  and the ratio to `lamai`, with no baseline subtraction. Without a `lamai`
  measurement the ratio is `n/a`.

Failure reports include the first mismatching run per implementation and
reference VM disassembly. Compiler disassembly is not printed, even with
`--disassemble`; that option still requests and reads the compiler listing.
`--show-stderr` prints the first run's stderr for each measured implementation.

## Variadic runtime fixtures

Applied test specification: `examples/StdVararg*` covers `assert`, `printf`,
`sprintf`, and `fprintf`. Each fixture has an `I,Std;` interface to select
VM-only comparison because the source interpreter lacks these native builtins.

- `Assert` checks true conditions, including a non-unit truth value, with
  empty and mixed variadic tails; reaching its final output proves continuation.
- `AssertFail` checks that a false condition exits with code 255 before any
  subsequent output. Stderr remains diagnostic only.
- `Printf` exercises an empty format, literal percent signs, signed integers,
  strings, characters, dynamic width and precision, and enough mixed arguments
  to exceed register argument capacity.
- `Sprintf` checks returned strings for empty, literal, mixed, dynamic-width,
  and many-argument calls against explicit expected values.
- `Fprintf` writes empty, literal, mixed, and many-argument calls to a file in
  the harness's temporary directory, closes it, and checks the exact contents.

## Import fixtures

Keep `.lama` sources and sibling `.i` interfaces together. The bytecode compiler
consumes interfaces but does not generate them. The compiler wrapper follows
`I,name;` declarations with sibling `name.lama` sources and compiles their
dependency closure into the same output directory. It caches a successful
compilation before visiting dependencies, allowing cycles to reach VM loading.
Imports without sibling sources are left to the configured library path.

`examples/OpFail` and `examples/OpFailKeep` also have interfaces to select VM-only
comparison. Both VMs report pattern-match failure with exit code 255, while
`lamai` raises an OCaml exception and exits with code 2. These fixtures exercise
the failure instructions and expect matching failure, rather than successful
execution.

| Fixtures | Purpose |
|:--|:--|
| `Leaf`, `Left`, `Right`, `Diamond`, `Reverse` | Shared dependencies and differing import orders |
| `Cycle` | Self-import rejection before execution |
| `Sum1`, `Sum2`, `Sum3` | Duplicate public function names and ambiguity rejection |
| `Sum4` | Calling an imported public function |
| `Sum5` | Imported global loads/stores and an independent local global |

These programs avoid unsupported runtime functions. Empty stdout alone does
not prove initialization order or value correctness. `Sum5` checks imported
initial values and writes explicitly; incorrect values take a division-by-zero
path. Inspect bytecode or use value-dependent failures when adding such checks.
The source interpreter does not provide an imported-unit initialization oracle,
which is why interface-bearing cases skip it.
