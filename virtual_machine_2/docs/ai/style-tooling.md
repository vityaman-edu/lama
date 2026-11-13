# C++ tooling specification

- Run clang-tidy in both `check-cpp.sh` and `style-cpp.sh` with up to eight
  parallel workers, preserving the existing launcher, arguments, and file set.
- Print `[clang-tidy] <path>` for each file and keep all diagnostics visible.
- Continue remaining checks and formatting after build, formatting, or
  clang-tidy failures, including a clang-tidy exit status of 255.
- Return a nonzero status after completing all work if any stage failed.
