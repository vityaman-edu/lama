#include "state.hpp"

#include "core/assert.hpp"

#include "interpreter/runtime/box.hpp"
#include "interpreter/runtime/state.hpp"

#include <cstddef>
#include <cstdint>
#include <iterator>
#include <span>

namespace lamavm::interpreter::cold {

const char* c_str(std::int32_t offset) {
  LAMAVM_EXPECT_BB(0, <=, offset, <, std::ssize(up->data));

  return up->data.data() + offset;
}

box& stack_push() {
  const void* stack_ptr = sp;
  const void* stack_begin = stack.data();
  const void* stack_end = stack.data() + stack.size();
  LAMAVM_EXPECT_BB(stack_begin, <, stack_ptr, <=, stack_end);

  return *--sp;
}

void stack_push(box x) {
  cold::stack_push() = x;
}

box stack_pop() {
  const void* stack_ptr = sp;
  const void* stack_begin = stack.data();
  const void* stack_end = stack.data() + stack.size();
  LAMAVM_EXPECT_BB(stack_begin, <=, stack_ptr, <, stack_end);

  return interpreter::stack_pop();
}

void stack_pop(std::int64_t count) {
  sp += stack_args(count).size();
}

std::span<box> stack_args(std::int64_t count) {
  LAMAVM_EXPECT_B(frame::size, <=, std::ssize(stack));

  const void* first_frame = stack.data() + (frame::size - 1);
  const void* frame_ptr = fp;
  const void* stack_end = stack.data() + stack.size();
  LAMAVM_EXPECT_BB(first_frame, <=, frame_ptr, <, stack_end);

  const box* frame_begin = fp - (frame::size - 1);
  const void* stack_ptr = sp;
  const void* stack_begin = stack.data();
  LAMAVM_EXPECT_BB(stack_begin, <=, stack_ptr, <=, static_cast<const void*>(frame_begin));
  LAMAVM_EXPECT_BB(0, <=, count, <=, frame_begin - sp);

  return {sp, static_cast<std::size_t>(count)};
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
