# Runtime split

Apply the requested separation of the runtime interface after moving the
interpreter into `source/interpreter/naive/`.

- Remove `source/interpreter/runtime.hpp` and its implementation file.
- Move `is_unboxed`, `box`, and `unbox` into paired `runtime/box.hpp` and
  `runtime/box.cpp`, preserving their signatures and tagged-value behavior.
- Move GC initialization and stack-boundary setters into paired `runtime/gc.hpp`
  and `runtime/gc.cpp`. Expose a non-instantiable `gc` class with static `init`,
  `set_stack_bottom`, and `set_stack_top` methods.
- Move all existing C-linkage operator and `Bstring` declarations into
  `runtime/std.hpp`. Pair it with `runtime/std.cpp`; definitions remain supplied
  by the existing C runtime library.
- Preserve the `lamavm::interpreter` namespace and update includes and GC calls.
- Validate formatting, Clang Tidy, release and sanitizer builds, and the existing
  differential example tests. Report unrelated test or lint failures separately.

## Validation

- Release and ASan/UBSan builds passed.
- All nine supported examples passed in both builds, with 20 runs per
  implementation per example.
- Formatting, whitespace checks, and Python checks passed. Every source `.hpp`
  has a paired `.cpp`.
- Clang Tidy rejects the two C runtime `__gc_stack_*` declarations as reserved
  identifiers and for naming style. Their old `NOLINT` comments were omitted to
  follow the source-comment restriction; no rules were disabled.
- Broader regression testing was stopped after confirmation that only examples
  are supported at this stage.
