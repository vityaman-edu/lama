#include "box.hpp"

extern "C" {
#include <contrib/runtime/runtime_common.h>
}

#include <bit>
#include <cstdint>

namespace lamavm::interpreter {

box::box() = default;

box::box(std::int64_t x)
  : underlying(BOX(x)) {
}

box::box(const void* x)
  : underlying(std::bit_cast<std::int64_t>(x)) {
}

box::operator std::int64_t() const {
  return UNBOX(underlying);
}

box::operator void*() const {
  return std::bit_cast<void*>(underlying);
}

} // namespace lamavm::interpreter
