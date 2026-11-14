#pragma once

#include <cstdint>
#include <type_traits>

namespace lamavm::interpreter {

class box final {
public:
  box();
  explicit box(std::int64_t x);
  explicit box(const void* x);

  explicit operator std::int64_t() const;
  explicit operator void*() const;

private:
  std::int64_t underlying = 0;
};

static_assert(sizeof(void*) == sizeof(box));
static_assert(std::is_standard_layout_v<box>);
static_assert(std::is_trivially_copyable_v<box>);

template <typename T>
T unbox(box x) {
  if constexpr (std::is_pointer_v<T>) {
    // NOLINTNEXTLINE(bugprone-casting-through-void)
    return static_cast<T>(static_cast<void*>(x));
  } else {
    return static_cast<T>(static_cast<std::int64_t>(x));
  }
}

} // namespace lamavm::interpreter
