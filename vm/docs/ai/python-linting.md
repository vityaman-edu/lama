# Python linting

Status: applied.

Provide Ruff in the Nix development shell. Add `scripts/test-python.sh` as a
standalone lint entry point and invoke it near the start of `scripts/test.sh`,
before checks that need built executables. Existing CI runs this through the
same script and needs no separate job.

The shell entry point locates `vm/` from its own path, checks the Python files
under `scripts/` and `test/`, and returns Ruff's exit status. It must work from
any working directory and with spaces in the checkout path, must not modify
files, and must not create a Ruff cache. Target Python 3.10. Enable an explicit,
broad set of stable lint families covering correctness, annotations, imports,
naming, security, unused arguments, modern syntax, paths, performance, and
idiomatic Python. Do not enable documentation requirements or complexity and
magic-number metrics for this wire-format comparison harness.

Require Ruff formatting with 100-column lines, double quotes, spaces, and LF
endings. The shell entry point runs both lint and non-mutating format checks.
Fix source violations rather than adding inline suppressions. Keep unsafe
automatic fixes disabled.

Exceptions are deliberately narrow: `main.py` prints CLI results and imports
local modules after setting `sys.dont_write_bytecode`; the two executable
adapters launch explicitly supplied compiler/VM paths without a shell; and the
dump parser handles heterogeneous recursive values requiring explicit `Any`
annotations. Allow only the five unittest override names that its API requires.

Keep all golden runner imports at the top level and preserve its early
bytecode-cache suppression without inline suppressions or nested imports.
Do not change the comparison behavior or add third-party runtime dependencies
to the golden harness.

Acceptance: the lint entry point passes on the checked-in Python sources from
the checkout and another working directory, and returns nonzero for a lint
error or a formatting violation. The golden runner still prints help and runs
its existing corpus.
