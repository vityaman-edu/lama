#include "state.hpp"

namespace lamavm::interpreter {

bc::symbol::index symbols;
std::vector<uctx> units;
std::vector<box> stack;

const op::word_t* pc = nullptr;
box* sp = nullptr;
box* fp = nullptr;
const uctx* up = nullptr;

} // namespace lamavm::interpreter
