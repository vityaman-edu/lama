#pragma once

#include "layout.hpp"

#include <cstddef>
#include <expected>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace lamavm::bc {

class file_listener {
public:
  virtual ~file_listener() = 0;

  virtual void operator()(const header &op) = 0;
  virtual void operator()(const import &op) = 0;
  virtual void operator()(const public_symbol &op) = 0;
  virtual void operator()(std::string_view strings) = 0;
  virtual auto operator()(std::span<const std::byte> code)
    -> std::expected<void, std::vector<std::string>> = 0;
};

} // namespace lamavm::bc
