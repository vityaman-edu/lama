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
