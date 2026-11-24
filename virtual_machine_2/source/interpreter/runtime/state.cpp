#include "state.hpp"

#include "box.hpp"
#include "gc.hpp"
#include "stack_ptr.hpp"

#include "bc/units.hpp"

#include "op/word.hpp"
#include <vector>

namespace lamavm::interpreter {

bc::symbol::index symbols;
std::vector<uctx> units;

op::word_t* pc = nullptr;
stack_ptr<box> sp(&gc::top()); // NOLINT(bugprone-throwing-static-initialization)
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

void stack_push_frame(frame f) {
  sp -= frame::size;
  fp = sp + (frame::size - 1);
  fp[frame::offset_n_args] = box(f.n_args);
  fp[frame::offset_fp] = box(f.fp);
  fp[frame::offset_pc] = box(f.pc);
  fp[frame::offset_sp] = box(f.sp);
  fp[frame::offset_up] = box(f.up);
}

void stack_pop_frame() {
  up = unbox<const uctx*>(fp[frame::offset_up]);
  sp = unbox<box*>(fp[frame::offset_sp]);
  pc = unbox<op::word_t*>(fp[frame::offset_pc]);
  fp = unbox<box*>(fp[frame::offset_fp]);
}

} // namespace lamavm::interpreter
