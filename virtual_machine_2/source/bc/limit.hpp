#pragma once

#include <cstddef>
#include <cstdint>

namespace lamavm {

static constexpr std::int32_t max_unit_size            = 1 * 1024 * 1024 * 1024;
static constexpr std::int32_t max_stack_size           = 8 * 1024 * 1024;
static constexpr std::int32_t max_globals_count        = max_stack_size / 4 / 2;
static constexpr std::int32_t max_imports_count        = 64 * 1024;
static constexpr std::int32_t max_public_symbols_count = 1024 * 1024;

} // namespace lamavm
