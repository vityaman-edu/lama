#pragma once

#include <cstdint>

namespace lamavm::runtime {

[[nodiscard]] auto box(std::int64_t value) -> std::int64_t;

[[nodiscard]] auto unbox(std::int64_t value) -> std::int64_t;

} // namespace lamavm::runtime
