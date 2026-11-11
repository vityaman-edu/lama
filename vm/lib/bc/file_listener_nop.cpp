#include "file_listener_nop.hpp"

#include "core/exception.hpp"
#include "layout.hpp"

#include <cstddef>
#include <expected>
#include <span>
#include <string>
#include <vector>

namespace lamavm::bc {

auto file_listener_nop::operator()(const header &op) -> status {
  (void)op;

  return {};
}

auto file_listener_nop::operator()(const import &op) -> status {
  (void)op;

  return {};
}

auto file_listener_nop::operator()(const public_symbol &op) -> status {
  (void)op;

  return {};
}

auto file_listener_nop::operator()(std::string &strings) -> status {
  (void)strings;

  return {};
}

auto file_listener_nop::operator()(std::span<const std::byte> code)
  -> std::expected<void, std::vector<error>> {
  (void)code;

  return {};
}

} // namespace lamavm::bc
