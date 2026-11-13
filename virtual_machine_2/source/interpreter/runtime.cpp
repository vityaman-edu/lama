#include "runtime.hpp"

#include <contrib/runtime/runtime_common.h>

#include <cstdint>

namespace lamavm::interpreter {

bool is_unboxed(std::int64_t x) {
  return UNBOXED(x);
}

std::int64_t box(std::int64_t x) {
  return BOX(x);
}

std::int64_t unbox(std::int64_t x) {
  return UNBOX(x);
}

} // namespace lamavm::interpreter
