#include "boot.hpp"

#include "bc/code_listener_nop.hpp"
#include "bc/code_reader.hpp"
#include "bc/layout.hpp"
#include "core/exception.hpp"

#include <cstddef>
#include <expected>
#include <span>
#include <string_view>
#include <vector>

namespace lamavm::interpreter {

auto boot::operator()(const bc::header &op) -> status {
  (void)op;

  return {};
}

auto boot::operator()(const bc::import &op) -> status {
  (void)op;

  return {};
}

auto boot::operator()(const bc::public_symbol &op) -> status {
  (void)op;

  return {};
}

auto boot::operator()(std::string_view strings) -> status {
  (void)strings;

  return {};
}

auto boot::operator()(std::span<const std::byte> code) -> std::expected<void, std::vector<error>> {
  bc::code_listener_nop listener;
  return bc::read_ops(code, listener);
}

} // namespace lamavm::interpreter
