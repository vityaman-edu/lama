#pragma once

#include <cstddef>
#include <filesystem>
#include <span>
#include <string_view>
#include <vector>

namespace lamavm::bc {

struct unit_t {
  const std::vector<char> buffer;
  std::string_view string_table;
  std::span<const std::byte> code;

  std::string_view string_at(size_t i);
};

unit_t parse_unit(std::vector<char> buffer);
unit_t parse_unit(std::istream &in);
unit_t parse_unit(const std::filesystem::path &path);

} // namespace lamavm::bc
