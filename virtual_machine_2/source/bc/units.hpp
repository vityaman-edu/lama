#pragma once

#include "unit.hpp"

#include <filesystem>
#include <map>
#include <string>
#include <unordered_map>
#include <vector>

namespace lamavm::bc {

struct units {
  using container = std::map<std::string, unit>;
  using iterator = container::iterator;

  struct symbol : public_symbol {
    iterator unit;
  };

  std::vector<iterator> initialization_order;
  container by_name;

  std::unordered_map<std::string_view, symbol> symbols;

  static units from(const std::filesystem::path& path);
};

} // namespace lamavm::bc
