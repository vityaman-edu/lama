# VM infrastructure

Restore infrastructure from commit `f73603d7a0f529de02e27af178b3102542213981`
without restoring the VM implementation.

- CMake supports C23, C++23, assembly, reusable library targets, Cxxopts, and
  release and ASan/UBSan configurations under `build/rel` and `build/san`.
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
- Ruff, C++ formatting, and Clang Tidy are exposed through Make targets.
- CI builds and checks release and sanitizer configurations and runs differential
  tests. A dummy application is expected to fail behavioral tests.
- On macOS, the development shell and sanitizer tests set `MallocNanoZone=0`
  before launching programs, preventing allocator reservation warnings under ASan.
- Restore `docs/hw` verbatim and link `SPEC.md` to the reference VM specification.
- Do not introduce FuzzTest, unit-test frameworks, or the previous C++ VM code.
