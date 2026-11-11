#include "state.hpp"

#include "stack.hpp"

#include <cstddef>
#include <span>
#include <string>
#include <utility>

namespace lamavm::interpreter {

state::state(std::string string_table, std::span<const std::byte> code)
  : area_string_table(std::move(string_table))
  , string_table(area_string_table)
  , code(code)
  , bp(stack::capacity()) {
  stack::acquire();
}

state::~state() {
  stack::release();
}

} // namespace lamavm::interpreter
