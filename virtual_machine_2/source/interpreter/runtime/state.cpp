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

void stack_push(box x) {
  --sp;
  *sp = x;
}

box stack_pop() {
  const box x = *sp;
  ++sp;
  return x;
}

void stack_pop_frame() {
  up = unbox<const uctx*>(fp[frame::offset_up]);
  sp = unbox<box*>(fp[frame::offset_sp]);
  pc = unbox<const op::word_t*>(fp[frame::offset_pc]);
  fp = unbox<box*>(fp[frame::offset_fp]);
}

std::int64_t stack_n_args() {
  return unbox<std::int64_t>(fp[frame::offset_n_args]);
}

} // namespace lamavm::interpreter
