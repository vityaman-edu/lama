#pragma once

#include "layout.hpp"

#include <cstddef>
#include <expected>
#include <span>
#include <string_view>
#include <variant>

namespace lamavm::bc {

class file_listener {
public:
  virtual ~file_listener() = 0;

  virtual void operator()(std::size_t offset, std::string_view message) = 0;
  virtual void operator()(const header &instruction) = 0;
  virtual void operator()(const import &instruction) = 0;
  virtual void operator()(const public_symbol &instruction) = 0;
  virtual void operator()(std::string_view strings) = 0;
  virtual auto operator()(std::span<const std::byte> code)
    -> std::expected<void, std::monostate> = 0;
};

} // namespace lamavm::bc
