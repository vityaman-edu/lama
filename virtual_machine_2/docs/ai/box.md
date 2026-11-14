# Box values

Apply the requested replacement of legacy integer boxing helpers with the
`lamavm::interpreter::box` class declared in `runtime/box.hpp`.

- Keep a single `std::int64_t` data member so each value occupies one runtime
  word. Default construction preserves the declared zero representation.
- Constructing from `std::int64_t` applies the runtime's `BOX` encoding;
  explicit const conversion to `std::int64_t` applies `UNBOX`.
- Remove the legacy `is_unboxed(std::int64_t)`, `box(std::int64_t)`, and
  `unbox(std::int64_t)` functions. Retain the new `unbox<T>(box)` helper.
- Change the operand stack to `std::vector<box>` and its stack/frame cursors to
  `box*`. Preserve frame layout, operand order, global/local/argument access,
  singleton initialization, GC bounds, and existing supported operations.
- Preserve raw pointer bits in frame and heap values, and pass already-tagged
  words across the C runtime boundary without applying integer boxing again.
  Add explicit construction from `const void*` and const conversion to `void*`;
  let `unbox<T>` cast that pointer to the requested pointer type.
- Apply the user's follow-up: `Bstring` takes and returns `box`, and all
  `Ls__Infix_*` functions take two boxes and return a box. Keep their existing
  C ABI behind C++ wrappers, using bit-preserving conversion for tagged
  integers. `Bstring` still receives a pointer to the string-argument slot.
- Assert `sizeof(void*) == sizeof(box)`, standard layout, and trivial
  copyability so the GC and runtime retain their word representation.

Validate formatting, focused Clang Tidy, release and ASan/UBSan builds, integer
and pointer representations, runtime arithmetic, calls, and all nine supported
examples.

## Validation

- Release and ASan/UBSan builds passed. All nine supported examples matched
  the reference implementations in both builds, with 20 runs per example.
- A temporary harness checked default representation, integer tag bits and
  round-trips including the signed 63-bit bounds, pointer round-trips, string
  allocation and contents, and all 13 arithmetic wrappers under ASan/UBSan.
- A negative arithmetic result passed in release, but UBSan reported the
  existing C runtime's left shift of a negative value in `runtime.c:171`.
  That runtime source was not changed.
- Formatting and whitespace checks passed. Focused Clang Tidy reports 27
  diagnostics for reserved identifiers and naming style on the required C
  runtime symbols. No rules were disabled and no suppression comments added.
