# Legacy bytecode disassembly golden tests

Status: proposed feature plan. Implementation follows review of this plan.
Paths below are relative to `vm/` unless stated otherwise.

## Goal

For each `.lama` source in the repository's `regression/` directory, expose an
individual GoogleTest case that compiles it with `lamac -b -ds`, reads the
resulting `.bc` through `lib/bc/input.hpp`, prints it through
`lib/bc/lama_ds_output.hpp`, and compares that output with the compiler's `.sm`
dump. The compiler dump is the golden, generated afresh for each case.

## Repository observations

- `test/golden/data-legacy` currently exists as an empty directory. Replace it
  with the relative symlink `../../../regression`; verify it is still empty
  before replacing it. Keep `test/golden/data/` for VM-specific fixtures.
- `regression/` currently contains 79 `.lama` files. Its `vm/` subdirectory holds
  Cram tests rather than additional sources. Discover files, not test numbers.
- The printer currently lives in `lib/bc/lamac_ds.hpp` and `.cpp`, with class
  `lamac_ds_listener`. Proposed naming change: rename those files to
  `lama_ds_output.hpp` and `.cpp` and update includes, retaining the class and
  behavior. This provides the header requested for the golden driver.
- `scripts/lamac.sh` is an executable wrapper around
  `_build/default/src/Driver.exe` with explicit runtime and include paths,
  resolved relative to the script. It forwards arguments and preserves the
  caller's working directory. The devshell's `lamac` function delegates to it.
- `-ds` writes `<stem>.sm`; `-b` writes `<stem>.bc`. Both go into the compiler's
  working directory, even when the input source has an absolute path.
- `cmake/Test.cmake` currently supports only unit and fuzz tests. The GoogleTest
  runner selects the `unit` CTest label, so golden tests need explicit wiring.

## Test driver and compiler invocation

1. Add `test/golden/driver_legacy.cpp`, linked with `liblamavm` and
   `GTest::gtest_main`. Define a parameterized suite with one comparison per
   source and a stable name such as `Legacy/Disassembly.MatchesCompiler/test001`.
2. Pass the absolute `scripts/lamac.sh` and corpus paths from CMake. Invoke the
   wrapper directly with an argument vector containing `-b -ds` and the absolute
   source path. The wrapper supplies runtime and include paths. Do not depend
   on a shell function or interpolate source paths into shell commands.
3. Create a unique temporary working directory for each case, including across
   concurrent test processes. Compile an absolute source path there. Set the
   child's working directory without changing the test process's directory.
   Capture stdout, stderr, exit status and signal termination; enforce a bounded
   compiler timeout. A missing compiler or corpus is a setup failure, not a skip.
4. Require successful compilation and newly generated `.sm` and `.bc` files.
   Open bytecode in binary mode, call `bc::read_file`, and collect the printer's
   output in an `ostringstream`. A listener diagnostic, failed read or exception
   fails the case before comparison.
5. Compare normalized compiler text against normalized printer text. On failure,
   report the source, command arguments, compiler diagnostics, parser offset and
   message where applicable, and a useful text difference. Keep raw and normalized
   outputs for failed cases under the build directory; clean successful cases.

Use a small process helper for the existing Linux/macOS targets; avoid adding a
process library just for this driver. This suite compiles sources and inspects
bytecode; it does not execute LaMa programs or consume `.input`/`.t` fixtures.
Do not inherit the existing VM Cram exclusion of `test803` without evidence that
it cannot be compiled for this comparison.

## Comparison contract

Raw equality is impossible: the compiler dump includes information the bytecode
does not serialize. Keep normalization in the test layer, with explicit rules
based on `src/SM.ml`; do not weaken the production printer to make tests pass.

- Read the compiler dump as structured instructions, respecting quoted strings,
  escaped bytes, lists and nested scope records. Reject unknown forms rather
  than ignoring unmatched lines or applying broad regular expressions.
- Resolve `LABEL`, `FLABEL` and `SLABEL` definitions to serialized code offsets
  using instruction sizes from the wire format. Preserve the actual destination
  of every jump, internal call and closure; distinguish external names from
  internal targets. Compare against the printer's hexadecimal target names.
  Remove source label declarations after resolution and handle the printer's
  synthetic function-entry labels consistently.
- Use `EXTERN` declarations to distinguish external references, then omit those
  declarations because they are not emitted as instructions. Map internal global
  names to indices in the compiler's allocation order, including globals in
  closure capture lists. Preserve external symbol names and local, argument and
  captured-variable indices.
- Normalize `BEGIN`'s source label, argument names and scope trees, and replace
  its capture designations with positional `Access` entries. Preserve argument,
  local and capture counts. Preserve the actual kind and index of every capture
  in `CLOSURE`, which are serialized.
- Normalize only the tail-call flag in `CALL` and `CALLC`, which bytecode omits.
  Preserve the `FAIL` keep-value flag, which has distinct opcodes. Account for
  `.array`'s dedicated encoding without erasing its arity.
- Decode OCaml dump quoting separately from the compiler's string-literal
  unescaping before bytecode serialization. Compare the resulting byte strings
  with identical escaping. Apply transformations only to fields where the
  compiler applies them; preserve string contents and tags as encoded.
- Compare imports and public declarations in their serialized table order, then
  the instruction stream. Preserve opcodes, order, signed operands, arities,
  line numbers, failure positions and all other serialized information.
- Treat EOF as the bytecode terminator rather than a compiler dump instruction.
  Normalize line endings and the final dump newline explicitly.

Do not derive expected operands from the decoded `.bc`: the compiler `.sm` must
remain the independent reference. Unexpected compiler/bytecode differences need
investigation and a documented rule or a separate defect; do not silently drop
instructions, operands, string differences or failing sources.

## Automatic GoogleTest cases and CMake integration

- Add `test/golden/CMakeLists.txt` and include it from `cmake/Test.cmake` when
  `BUILD_TESTING` is enabled. Add a golden-test helper alongside the existing
  unit/fuzz helpers, linking GoogleTest and labeling discovered cases `golden`.
- At configure time, enumerate `data-legacy/*.lama` with CMake
  `file(GLOB ... CONFIGURE_DEPENDS ...)`, sort the paths, and generate a small
  manifest header in the build directory. The corpus is currently flat; recurse
  only if its layout changes. Fail configuration for a broken symlink or an
  empty corpus.
- Use `TEST_P` and `INSTANTIATE_TEST_SUITE_P` with `ValuesIn` over that manifest.
  Derive valid, unique GoogleTest parameter names from relative source paths;
  detect naming collisions. No handwritten per-file tests or external source
  generator is needed.
- Use `gtest_discover_tests` so CTest can list, filter and run each source
  separately. Discovery must only register cases: it must not spawn the compiler
  or create test artifacts. A changed manifest rebuilds the driver and refreshes
  CTest discovery after sources are added or removed.
- Add the driver to `lamavm_tests`. Extend `scripts/googletest.sh` to select both
  `unit` and `golden` labels so `scripts/test.sh` and existing CI run the suite.
- Ensure `scripts/build.sh` builds the compiler and required interface files
  before golden execution. Verify the existing runtime and stdlib paths; request
  only missing Dune targets. `BUILD_TESTING=OFF` must not require the compiler or
  inspect the corpus.

## Implementation order and acceptance

1. Rename the printer files and update existing consumers; add the corpus
   symlink, target and automatic registration. Verify discovery lists all 79
   current sources without compiling them.
2. Add isolated compiler execution and parser/printer integration. Establish the
   comparison contract first on a simple source, then globals, branches,
   functions, closures, pattern matching and escaped strings from the corpus.
3. Complete normalization and run every generated case. Investigate every
   mismatch before expanding normalization; record any actual defects separately.
4. Verify individual filtering, parallel CTest execution, paths containing
   spaces, and failure diagnostics. Confirm that adding/removing a temporary
   corpus source and rebuilding updates discovery, then remove that fixture.
5. Check that changing a serialized operand, target or string in a temporary
   artifact causes comparison failure. Add focused tests for normalization only
   where they demonstrate that meaningful differences remain detectable.
6. Run existing bytecode unit tests and the new suite in the normal devshell;
   run the project's formatting and analysis checks for changed C++ files.
   Confirm no generated artifacts appear in `regression/` or other source
   directories. Keep implementation changes within the mutable repository paths.
