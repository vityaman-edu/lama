#include "state.hpp"
#include "bc/units.hpp"
#include "interpreter/runtime/box.hpp"
#include "op/word.hpp"
#include <cstdint>
#include <vector>

namespace lamavm::interpreter {

bc::symbol::index symbols;
std::vector<uctx> units;
std::vector<box> stack;

const op::word_t* pc = nullptr;
box* sp = nullptr;
box* fp = nullptr;
const uctx* up = nullptr;

const char* c_str(std::int32_t offset) {
  return up->data.data() + offset;
}

} // namespace lamavm::interpreter
