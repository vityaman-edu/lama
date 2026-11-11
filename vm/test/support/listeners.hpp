#pragma once

#include "bc/file_listener.hpp"
#include "bc/layout.hpp"
#include "core/exception.hpp"

#include <cstddef>
#include <expected>
#include <span>
#include <string>
#include <vector>

namespace lamavm::bc::test {

class noop_file_listener : public file_listener {
public:
  using file_listener::operator();

  auto operator()(const header &op) -> status override;
  auto operator()(const import &op) -> status override;
  auto operator()(const public_symbol &op) -> status override;
  auto operator()(std::string &strings) -> status override;
  auto operator()(std::span<const std::byte> code)
    -> std::expected<void, std::vector<error>> override;
};
} // namespace lamavm::bc::test
