#include "listeners.hpp"

#include "bc/layout.hpp"
#include "core/exception.hpp"

#include <cstddef>
#include <expected>
#include <span>
#include <string>
#include <vector>

namespace lamavm::bc::test {

auto noop_file_listener::operator()(const header &op) -> status {
  (void)op;

  return {};
}

auto noop_file_listener::operator()(const import &op) -> status {
  (void)op;

  return {};
}

auto noop_file_listener::operator()(const public_symbol &op) -> status {
  (void)op;

  return {};
}

auto noop_file_listener::operator()(std::string &strings) -> status {
  (void)strings;

  return {};
}

auto noop_file_listener::operator()(std::span<const std::byte> code)
  -> std::expected<void, std::vector<error>> {
  (void)code;

  return {};
}

} // namespace lamavm::bc::test
