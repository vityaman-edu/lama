#include "box.hpp"

#include <runtime_common.h>

#include <cstdint>

namespace lamavm::runtime {

auto box(std::int64_t value) -> std::int64_t {
  return BOX(value);
}

auto unbox(std::int64_t value) -> std::int64_t {
  return UNBOX(value);
}

} // namespace lamavm::runtime
