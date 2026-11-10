#include "listeners.hpp"

#include "bc/layout.hpp"

#include <cstddef>
#include <expected>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace lamavm::bc::test {

void noop_file_listener::operator()(const header &op) {
  (void)op;
}

void noop_file_listener::operator()(const import &op) {
  (void)op;
}

void noop_file_listener::operator()(const public_symbol &op) {
  (void)op;
}

void noop_file_listener::operator()(std::string_view strings) {
  (void)strings;
}

auto noop_file_listener::operator()(std::span<const std::byte> code)
  -> std::expected<void, std::vector<std::string>> {
  (void)code;

  return {};
}

} // namespace lamavm::bc::test
