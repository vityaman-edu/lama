#pragma once

#include <cstdint>

namespace lamavm::interpreter {

bool is_unboxed(std::int64_t x);

std::int64_t box(std::int64_t x);

std::int64_t unbox(std::int64_t x);

} // namespace lamavm::interpreter
