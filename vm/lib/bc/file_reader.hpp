#pragma once

#include "file_listener.hpp"

#include <expected>
#include <iosfwd>
#include <variant>

namespace lamavm::bc {

auto read_file(std::istream &in, file_listener &listener) -> std::expected<void, std::monostate>;

} // namespace lamavm::bc
