#pragma once

#include "core/exception.hpp"
#include "file_listener.hpp"

#include <expected>
#include <iosfwd>
#include <vector>

namespace lamavm::bc {

auto read_file(std::istream &in, file_listener &listener)
  -> std::expected<void, std::vector<error>>;

} // namespace lamavm::bc
