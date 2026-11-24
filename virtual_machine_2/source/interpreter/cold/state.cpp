#include "state.hpp"

#include "core/assert.hpp"

#include "interpreter/runtime/box.hpp"
#include "interpreter/runtime/state.hpp"
#include "op/word.hpp"

#include <cstddef>
#include <cstdint>
#include <iterator>
#include <span>
#include <tuple>
#include <vector>

namespace lamavm::interpreter::cold {

std::size_t globals_count = 0;

namespace {

box* frame_begin() {
  LAMAVM_EXPECT(fp != nullptr);
  LAMAVM_EXPECT_B(frame::size, <=, std::ssize(stack));

  const void* first_frame = stack.data() + (frame::size - 1);
  const void* frame_ptr = fp;
  const void* stack_end = stack.data() + stack.size();
  LAMAVM_EXPECT_BB(first_frame, <=, frame_ptr, <, stack_end);

  return fp - (frame::size - 1);
}

} // namespace

const char* c_str(std::int32_t offset) {
  LAMAVM_EXPECT_BB(0, <=, offset, <, std::ssize(up->data));

  return up->data.data() + offset;
}

box& stack_push() {
  const void* stack_ptr = static_cast<box*>(sp);
  const void* stack_begin = stack.data();
  const void* stack_end = stack.data() + stack.size();
  LAMAVM_EXPECT_BB(stack_begin, <, stack_ptr, <=, stack_end);

  return *--sp;
}

void stack_push(box x) {
  cold::stack_push() = x;
}

box stack_pop() {
  const void* stack_ptr = static_cast<box*>(sp);
  const void* stack_begin = stack.data();
  const void* stack_end = stack.data() + stack.size();
  LAMAVM_EXPECT_BB(stack_begin, <=, stack_ptr, <, stack_end);

  return interpreter::stack_pop();
}

void stack_pop(std::int64_t count) {
  sp += std::ssize(stack_args(count));
}

std::span<box> stack_args(std::int64_t count) {
  const auto* frame_start = frame_begin();
  auto* args = static_cast<box*>(sp);
  const void* stack_ptr = args;
  const void* stack_begin = stack.data();
  LAMAVM_EXPECT_BB(stack_begin, <=, stack_ptr, <=, static_cast<const void*>(frame_start));
  LAMAVM_EXPECT_BB(0, <=, count, <=, frame_start - args);

  return {args, static_cast<std::size_t>(count)};
}

box stack_pop_arg() {
  std::ignore = stack_args(1);
  return interpreter::stack_pop();
}

void stack_push_frame(frame f) {
  LAMAVM_EXPECT_B(frame::size, <=, static_cast<box*>(sp) - stack.data());
  interpreter::stack_push_frame(f);
}

void stack_init_locals(std::int32_t n_locals) {
  const auto* begin = frame_begin();
  const void* stack_ptr = static_cast<box*>(sp);
  LAMAVM_EXPECT_B(stack_ptr, ==, static_cast<const void*>(begin));
  LAMAVM_EXPECT_BB(0, <=, n_locals, <=, begin - stack.data());

  for (std::int32_t i = 0; i < n_locals; ++i) {
    cold::stack_push(box(std::int64_t{0}));
  }
}

std::int32_t stack_n_args() {
  return unbox<std::int32_t>(fp[frame::offset_n_args]);
}

const box* stack_frame_closure() {
  std::ignore = frame_begin();
  const auto offset = std::int64_t{stack_n_args()} + 1;
  const auto* saved_sp = unbox<box*>(fp[frame::offset_sp]);
  LAMAVM_EXPECT_C(offset == saved_sp - fp - 1, "current frame has no closure slot");

  return fp + offset;
}

} // namespace lamavm::interpreter::cold
