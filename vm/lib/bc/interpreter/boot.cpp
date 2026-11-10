#include "boot.hpp"

#include "bc/code_listener_nop.hpp"
#include "bc/code_reader.hpp"
#include "bc/layout.hpp"

#include <cstddef>
#include <expected>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace lamavm::bc::interpreter {

void boot::operator()(const header &op) {
  (void)op;
}

void boot::operator()(const import &op) {
  (void)op;
}

void boot::operator()(const public_symbol &op) {
  (void)op;
}

void boot::operator()(std::string_view strings) {
  (void)strings;
}

auto boot::operator()(std::span<const std::byte> code)
  -> std::expected<void, std::vector<std::string>> {
  code_listener_nop listener;
  return read_ops(code, listener);
}

} // namespace lamavm::bc::interpreter
