#pragma once

#include "bc/file_listener.hpp"
#include "bc/layout.hpp"
#include "core/exception.hpp"

#include <cstddef>
#include <expected>
#include <span>
#include <string>
#include <vector>

namespace lamavm::interpreter {

class boot : public bc::file_listener {
public:
  using bc::file_listener::operator();

  auto operator()(const bc::header &op) -> status override;

  auto operator()(const bc::import &op) -> status override;

  auto operator()(const bc::public_symbol &op) -> status override;

  auto operator()(std::string &strings) -> status override;

  auto operator()(std::span<const std::byte> code)
    -> std::expected<void, std::vector<error>> override;

private:
  std::string string_table;
};

} // namespace lamavm::interpreter
