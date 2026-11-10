#pragma once

#include "code_listener.hpp"

#include <cstddef>
#include <expected>
#include <span>
#include <variant>

namespace lamavm::bc {

auto read_code(std::span<const std::byte> code, code_listener &listener)
  -> std::expected<void, std::monostate>;

} // namespace lamavm::bc
