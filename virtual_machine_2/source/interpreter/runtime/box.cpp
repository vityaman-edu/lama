#include "box.hpp"

#include "core/assert.hpp"

extern "C" {
#include <contrib/runtime/runtime_common.h>
}

#include <bit>
#include <cstdint>
#include <limits>

namespace lamavm::interpreter {

namespace {

constexpr auto min = std::numeric_limits<std::int64_t>::min() / 2;
constexpr auto max = std::numeric_limits<std::int64_t>::max() / 2;

} // namespace

box::box() = default;

box::box(std::int64_t x) {
  LAMAVM_ASSERT_BB(min, <=, x, <=, max);

  underlying = BOX(x);
}

box::box(const void* x)
  : underlying(std::bit_cast<std::int64_t>(x)) {
}

box box::checked(std::int64_t x) {
  LAMAVM_EXPECT_BB(min, <=, x, <=, max);
  return box(x);
}

box::operator std::int64_t() const {
  return UNBOX(underlying);
}

box::operator void*() const {
  return std::bit_cast<void*>(underlying);
}

bool box::is_ptr() const {
  return !UNBOXED(underlying);
}

} // namespace lamavm::interpreter
