#include "state.hpp"

#include <cstddef>
#include <string>
#include <utility>
#include <vector>

namespace lamavm::interpreter {

state::state(std::string string_table, std::vector<std::byte> stack)
  : area_string_table(std::move(string_table))
  , area_stack(std::move(stack))
  , string_table(area_string_table)
  , stack(area_stack) {
}

} // namespace lamavm::interpreter
