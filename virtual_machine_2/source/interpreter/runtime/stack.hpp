#pragma once

#include "os/mmap.hpp"

#include <cstdint>

namespace lamavm::interpreter {

[[nodiscard]] os::mmap map_stack();

[[nodiscard]] bool is_top_stack_guard(std::uintptr_t address) noexcept;
[[nodiscard]] bool is_bot_stack_guard(std::uintptr_t address) noexcept;

} // namespace lamavm::interpreter
