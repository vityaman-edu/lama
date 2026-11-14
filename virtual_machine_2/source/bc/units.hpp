#pragma once

#include "unit.hpp"

#include <cstddef>
#include <filesystem>
#include <limits>
#include <map>
#include <string>
#include <unordered_map>
#include <vector>

namespace lamavm::bc {

struct symbol : public_symbol {
  using index = std::unordered_map<std::string_view, symbol>;

  // by initialization_order
  std::size_t uidx = std::numeric_limits<std::size_t>::max();
};

struct units {
  using container = std::map<std::string, unit>;
  using iterator = container::iterator;

  std::vector<iterator> initialization_order;
  container by_name;

  symbol::index symbols;

  static units from(const std::filesystem::path& path);
};

} // namespace lamavm::bc
