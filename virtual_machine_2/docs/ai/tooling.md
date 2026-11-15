# Build and checks

Run these commands from `virtual_machine_2`:

| Command | Purpose |
|:--|:--|
| `make lamavm2` | Release build in `build/rel` |
| `make lamavm2-san` | Debug build with ASan/UBSan in `build/san` |
| `make lamavm2-san-trace` | Sanitized build with VM tracing |
| `make lamac lamavm` | Prepare compiler wrapper and reference VM |
| `make compile_commands` | Configure release compilation database and its symlink |
| `make check-cpp` / `make check-py` | C++ / Python checks |
| `make style-cpp` / `make style-py` | Apply formatting and available fixes |
| `make test` / `make test-san` | Checks, prerequisites, build, and differential tests |

The build uses C++23 and an x86-64 runtime target, including on macOS.
[`scripts/lamavm2.sh`](../../scripts/lamavm2.sh) configures and builds the
selected mode, then points `lamavm2` at that binary. `LAMAVM_BUILD_JOBS` controls
build parallelism and defaults to two. [The test wrapper](testing.md) selects
the release or sanitized binary explicitly.

## External dependencies

CMake downloads pinned upstream dependencies into the build directory.
`cmake/Cxxopts.cmake` provides `cxxopts::cxxopts` through FetchContent.
`cmake/Libffi.cmake` builds the SHA-256-pinned libffi 3.8.1 release with
ExternalProject and exposes its static archive and system headers through
`libffi::libffi`. Both targets are private dependencies of `lamavm2`.
Libffi headers are available as `<contrib/libffi/ffi.h>` through a build-local
include tree; the target also provides the include path for `ffitarget.h`.

Libffi uses its upstream Autotools build and requires GNU Make. The release
archive includes `configure`, so Autotools need not be installed. Its build and
installation stay inside the CMake build directory. It uses the selected C
compiler and build-type flags; on macOS it targets x86-64 with the selected SDK.
Adding the dependency does not change VM call behavior.

## C++ checks

`check-cpp.sh` builds release, checks formatting, and runs Clang Tidy on source
translation units with up to eight workers. `style-cpp.sh` builds, runs Clang
Tidy with `-fix`, then formats source files. Both print `[clang-tidy] <path>`,
keep diagnostics visible, continue subsequent stages after failures, and return
nonzero if any stage failed, including Clang Tidy status 255.

`scripts/clang-tidy.sh` preserves the platform launcher behavior: on macOS it
clears `DEVELOPER_DIR` and `SDKROOT` and prefers `clang-tidy-unwrapped` when
available. Use that wrapper for focused checks as well.

Report validation for the current change separately from existing diagnostics.
Old successful runs do not establish that a later revision passes. Follow
`AGENTS.md` for source style, comments, and Clang Tidy restrictions.
