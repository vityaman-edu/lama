# VM infrastructure

Restore infrastructure from commit `f73603d7a0f529de02e27af178b3102542213981`
without restoring the VM implementation.

- CMake supports C23, C++23, assembly, reusable library targets, Cxxopts, and
  release and ASan/UBSan configurations under `build/rel` and `build/san`.
- Release uses `-O3` and full LTO for application and runtime C/C++ sources.
  Sanitizer builds use `-O0` without LTO.
- The root `CMakeLists.txt` defines `lamavm2` with a single dummy `main.cpp`.
  Project includes are relative to `source/`; future helpers live in `source/lib`.
- Runtime C and assembly sources come from the `contrib/runtime` symlink and
  form a separate `lama_runtime` library defined in `contrib/CMakeLists.txt`.
  The application is otherwise monolithic.
- Make targets invoke scripts in `scripts/`; aggregate targets use prerequisites.
  Build and test scripts accept an optional `rel` or `san` argument, defaulting
  to `rel`. Both VM builds update `lamavm2`
  to point to the selected release or sanitizer binary.
- Python files in `test/` discover `.lama` programs through `test/data/examples`
  and the `test/data/regression` symlink. Compilation uses temporary directories.
  The reference and candidate VMs receive identical bytes from sibling `.input`
  files; missing input files mean empty stdin. Execution differences fail tests.
- `test/lama.py` defines the common program execution interface and captures
  stdout, stderr, exit code, and execution time. `lamai.py`, `lamavm.py`, and
  `lamavm2.py` implement the interpreter and two VMs. `lamac.py` compiles once
  per case and can capture compiler disassembly; the reference VM also exposes
  disassembly. Timings measure process execution, including startup, and exclude
  compilation for the VMs. `--disassemble` adds both listings to failure reports.
  Benchmarks invoke the reference executable directly, supplying the standard
  library path as arguments, rather than timing its shell wrapper.
- `test/benchmark.py` measures a configurable number of runs and reports mean
  execution time with an approximate 95% bootstrap confidence interval.
  `--runs N` defaults to 20 and requires at least two samples. Intervals use
  2,000 bootstrap resamples with a fixed random seed. Report absolute wall times
  and each mean divided by the interpreter mean (`lamai` is `1.000x`). Ratios
  below one mean less elapsed time. Missing or zero interpreter timing produces
  an unavailable ratio. Do not subtract startup costs.
  Each case reports status and timings for all implementations; failures show
  program and stdin paths, captured stdout, stderr, and exit codes. Raw execution
  results are compared, including consistency between repeated runs.
- Result columns are aligned. Terminal output colors statuses, numeric values,
  and test names; redirected output stays plain text. `NO_COLOR` and `TERM=dumb`
  disable colors.
- `test/args.py` owns CLI parsing, validation, and filtered test discovery.
- Ruff, C++ formatting, and Clang Tidy are exposed through Make targets.
- CI builds and checks release and sanitizer configurations and runs differential
  tests. A dummy application is expected to fail behavioral tests.
- On macOS, the development shell and sanitizer tests set `MallocNanoZone=0`
  before launching programs, preventing allocator reservation warnings under ASan.
- Restore `docs/hw` verbatim and link `SPEC.md` to the reference VM specification.
- Do not introduce FuzzTest, unit-test frameworks, or the previous C++ VM code.
