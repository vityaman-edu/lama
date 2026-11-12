# Style tooling

Repair the malformed YAML keys in `.clang-tidy` without changing the enabled
checks or naming conventions.

Both C++ style and check scripts must use a shared Clang-Tidy launcher. On
macOS, clear `DEVELOPER_DIR` and `SDKROOT` and prefer `clang-tidy-unwrapped`
when available, so the Nix wrapper cannot inject headers that conflict with
the Apple SDK in the compilation database. Otherwise use `clang-tidy`.
Keep the existing build, formatting, and Python commands. In the style fix
pass, retain the configured warnings-as-errors policy and fail on diagnostics,
just as the check pass does. Compiler warnings must also fail the VM build
through `-Werror`. Build and Clang-Tidy failures must propagate to Make.

Validate the Clang-Tidy configuration, shell syntax, and analysis against the
release compilation database. Run the style target in a temporary copy to
avoid rewriting source files being edited concurrently. Report remaining
source diagnostics separately from tooling failures.
