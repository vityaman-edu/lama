#pragma once

#include "unit.hpp"

#include <filesystem>
#include <map>
#include <string>
#include <vector>

namespace lamavm::bc {

struct units {
  using container = std::map<std::string, unit>;
  using iterator = container::iterator;

  std::vector<iterator> initialization_order;
  container by_name;

  static units from(const std::filesystem::path& path);
};

} // namespace lamavm::bc
