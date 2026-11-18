#include "state.hpp"

#include "core/assert.hpp"

#include "interpreter/runtime/box.hpp"
#include "interpreter/runtime/state.hpp"

namespace lamavm::interpreter::cold {

void stack_push(box x) {
  const void* stack_ptr = sp;
  const void* stack_begin = stack.data();
  const void* stack_end = stack.data() + stack.size();
  LAMAVM_EXPECT_BB(stack_begin, <, stack_ptr, <=, stack_end);

  interpreter::stack_push(x);
}

box stack_pop() {
  const void* stack_ptr = sp;
  const void* stack_begin = stack.data();
  const void* stack_end = stack.data() + stack.size();
  LAMAVM_EXPECT_BB(stack_begin, <=, stack_ptr, <, stack_end);

  return interpreter::stack_pop();
}

box stack_pop_arg() {
  LAMAVM_EXPECT_B(static_cast<const void*>(fp), !=, nullptr);

  const void* frame_begin = fp - (frame::size - 1);
  LAMAVM_EXPECT_B(static_cast<const void*>(sp), <, frame_begin);

  return stack_pop();
}

void stack_push_frame(frame f) {
  cold::stack_push(box(f.n_args));
  box* new_fp = sp;
  cold::stack_push(box(f.fp));
  cold::stack_push(box(f.pc));
  cold::stack_push(box(f.sp));
  cold::stack_push(box(f.up));
  fp = new_fp;
}

} // namespace lamavm::interpreter::cold
