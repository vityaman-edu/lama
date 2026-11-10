#pragma once

#include "bc/file_listener.hpp"
#include "bc/layout.hpp"

#include <cstddef>
#include <expected>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace lamavm::interpreter {

class boot : public bc::file_listener {
public:
  using bc::file_listener::operator();

  void operator()(const bc::header &op) override;
  void operator()(const bc::import &op) override;
  void operator()(const bc::public_symbol &op) override;
  void operator()(std::string_view strings) override;
  auto operator()(std::span<const std::byte> code)
    -> std::expected<void, std::vector<std::string>> override;
};

} // namespace lamavm::interpreter
