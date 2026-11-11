#include "state.hpp"

#include "core/exception.hpp"

#include <cstddef>
#include <cstdint>
#include <expected>
#include <span>
#include <string>
#include <utility>
#include <vector>

namespace lamavm::interpreter {

state::state(
  std::string string_table,
  std::span<const std::byte> code,
  std::vector<std::int64_t> stack)
  : area_string_table(std::move(string_table))
  , area_stack(std::move(stack))
  , string_table(area_string_table)
  , code(code)
  , stack(area_stack) {
}

auto state::stack_push(std::int64_t value) -> status {
  if (sp == 0) {
    return std::unexpected("stack overflow");
  }

  stack.subspan(--sp).front() = value;
  return {};
}

auto state::stack_pop() -> std::expected<std::int64_t, error> {
  if (sp == stack.size()) {
    return std::unexpected("stack underflow");
  }

  return stack.subspan(sp++).front();
}

auto state::stack_top() const -> std::expected<std::int64_t, error> {
  if (sp == stack.size()) {
    return std::unexpected("stack underflow");
  }

  return stack.subspan(sp).front();
}

} // namespace lamavm::interpreter
