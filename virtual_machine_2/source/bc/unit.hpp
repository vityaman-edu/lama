#pragma once

#include "op/word.hpp"

#include <cstddef>
#include <filesystem>
#include <istream>
#include <unordered_map>
#include <vector>

namespace lamavm::bc {

struct public_symbol {
  enum class kind : std::uint8_t {
    function,
    global,
  };

  kind kind;
  std::int32_t offset;
};

struct unit {
  std::vector<char> string_table;
  std::vector<op::word_t> text;
  std::size_t globals_count = 0;
  std::vector<std::string_view> imports;
  std::unordered_map<std::string_view, public_symbol> public_symbols;

  static std::size_t size(const unit& u);

  static unit parse(std::istream& in);
  static unit parse(const std::filesystem::path& path);
};

} // namespace lamavm::bc
