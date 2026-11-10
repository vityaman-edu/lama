#include "file_listener_nop.hpp"

#include "layout.hpp"

#include <cstddef>
#include <expected>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace lamavm::bc {

void file_listener_nop::operator()(const header &op) {
  (void)op;
}

void file_listener_nop::operator()(const import &op) {
  (void)op;
}

void file_listener_nop::operator()(const public_symbol &op) {
  (void)op;
}

void file_listener_nop::operator()(std::string_view strings) {
  (void)strings;
}

auto file_listener_nop::operator()(std::span<const std::byte> code)
  -> std::expected<void, std::vector<std::string>> {
  (void)code;

  return {};
}

} // namespace lamavm::bc
