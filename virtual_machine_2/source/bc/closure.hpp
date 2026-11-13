#pragma once

#include "unit.hpp"

#include <string>
#include <unordered_map>
#include <vector>

namespace lamavm::bc {

struct preclosure {
  std::vector<std::string_view> order;
  std::unordered_map<std::string, unit> unit_by_name;

  static preclosure open(const std::filesystem::path& path);
};

unit closure(preclosure units);

} // namespace lamavm::bc
