#include "boot.hpp"

#include "bc/code_listener_nop.hpp"
#include "bc/code_reader.hpp"
#include "bc/layout.hpp"
#include "core/exception.hpp"
#include "loop.hpp"

#include <cstddef>
#include <expected>
#include <format>
#include <span>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace lamavm::interpreter {

auto boot::operator()(const bc::header &op) -> status {
  (void)op;

  return {};
}

auto boot::operator()(const bc::import &op) -> status {
  const auto start = static_cast<std::size_t>(op.name_offset);
  const auto end = string_table.find('\0', start);
  const auto name = std::string_view(string_table).substr(start, end - start);
  if (name != "Std") {
    return std::unexpected(std::format("unsupported import: {}", name));
  }

  return {};
}

auto boot::operator()(const bc::public_symbol &op) -> status {
  (void)op;

  return std::unexpected("public symbols are not supported");
}

auto boot::operator()(std::string &strings) -> status {
  string_table = std::move(strings);

  return {};
}

auto boot::operator()(std::span<const std::byte> code) -> std::expected<void, std::vector<error>> {
  bc::code_listener_nop listener;
  if (auto result = bc::read_ops(code, listener); !result) {
    return result;
  }

  loop machine(std::move(string_table), code);
  if (auto result = machine.run(); !result) {
    return std::unexpected(std::vector<error>{std::move(result.error())});
  }

  return {};
}

} // namespace lamavm::interpreter
