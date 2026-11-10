#pragma once

#include "file_listener.hpp"

#include <expected>
#include <iosfwd>
#include <string>
#include <vector>

namespace lamavm::bc {

auto read_file(std::istream &in, file_listener &listener)
  -> std::expected<void, std::vector<std::string>>;

} // namespace lamavm::bc
