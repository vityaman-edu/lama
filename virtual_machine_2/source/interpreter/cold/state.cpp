#include "state.hpp"

#include "core/assert.hpp"

#include "interpreter/runtime/box.hpp"
#include "interpreter/runtime/state.hpp"

#include "op/word.hpp"

#include <cstdint>

namespace lamavm::interpreter::cold {

void stack_push(box x) {
  const void* stack_ptr = sp;
  const void* stack_begin = stack.data();
  const void* stack_end = stack.data() + stack.size();
  LAMAVM_EXPECT_BB(stack_begin, <, stack_ptr, <=, stack_end);

  --sp;
  *sp = x;
}

box stack_pop() {
  const void* stack_ptr = sp;
  const void* stack_begin = stack.data();
  const void* stack_end = stack.data() + stack.size();
  LAMAVM_EXPECT_BB(stack_begin, <=, stack_ptr, <, stack_end);

  const box x = *sp;
  ++sp;
  return x;
}

box stack_pop_arg() {
  LAMAVM_EXPECT_B(static_cast<const void*>(fp), !=, nullptr);

  const void* frame_begin = fp - (frame::size - 1);
  LAMAVM_EXPECT_B(static_cast<const void*>(sp), <, frame_begin);

  return stack_pop();
}

void stack_push_frame(frame f) {
  stack_push(box(f.n_args));
  box* new_fp = sp;
  stack_push(box(f.fp));
  stack_push(box(f.pc));
  stack_push(box(f.sp));
  stack_push(box(f.up));
  fp = new_fp;
}

void stack_pop_frame() {
  up = unbox<const uctx*>(fp[-4]);
  sp = unbox<box*>(fp[-3]);
  pc = unbox<const op::word_t*>(fp[-2]);
  fp = unbox<box*>(fp[-1]);
}

std::int64_t stack_n_args() {
  return unbox<std::int64_t>(fp[0]);
}

} // namespace lamavm::interpreter::cold
