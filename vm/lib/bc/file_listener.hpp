#pragma once

#include "core/exception.hpp"
#include "layout.hpp"

#include <cstddef>
#include <expected>
#include <span>
#include <string_view>
#include <vector>

namespace lamavm::bc {

class file_listener {
public:
  virtual ~file_listener() = 0;

  virtual auto operator()(const header &op) -> status = 0;
  virtual auto operator()(const import &op) -> status = 0;
  virtual auto operator()(const public_symbol &op) -> status = 0;
  virtual auto operator()(std::string_view strings) -> status = 0;
  virtual auto operator()(std::span<const std::byte> code)
    -> std::expected<void, std::vector<error>> = 0;
};

} // namespace lamavm::bc
