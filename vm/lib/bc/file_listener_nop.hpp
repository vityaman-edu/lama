#pragma once

#include "core/exception.hpp"
#include "file_listener.hpp"
#include "layout.hpp"

#include <cstddef>
#include <expected>
#include <span>
#include <string>
#include <vector>

namespace lamavm::bc {

class file_listener_nop : public file_listener {
public:
  using file_listener::operator();

  auto operator()(const header &op) -> status override;
  auto operator()(const import &op) -> status override;
  auto operator()(const public_symbol &op) -> status override;
  auto operator()(std::string &strings) -> status override;
  auto operator()(std::span<const std::byte> code)
    -> std::expected<void, std::vector<error>> override;
};

} // namespace lamavm::bc
