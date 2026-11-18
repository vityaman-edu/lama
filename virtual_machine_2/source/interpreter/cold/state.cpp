#include "state.hpp"

#include "core/assert.hpp"

#include "interpreter/runtime/box.hpp"
#include "interpreter/runtime/state.hpp"
#include "op/word.hpp"

#include <cstddef>
#include <cstdint>
#include <iterator>
#include <span>
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

op::word_t* return_pc() {
  return unbox<op::word_t*>(fp[frame::offset_pc]);
}

const op::word_t* frame_entry() {
  const auto* continuation = return_pc();
  if (continuation[-1].as_call.unit_index != op::call_t::indirect) {
    return continuation[-2].as_target;
  }

  const auto* caller_sp = unbox<box*>(fp[frame::offset_sp]);
  const auto* closure = unbox<box*>(caller_sp[-1]);
  const auto* callee = unbox<const uctx*>(closure[1]);
  const auto offset = unbox<std::int32_t>(closure[0]);
  return callee->text.data() + offset;
}

} // namespace

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
  const auto n_locals = stack_n_locals();
  const box* operands_end = fp - (frame::size - 1) - n_locals;
  const void* stack_ptr = sp;
  const void* stack_begin = stack.data();
  LAMAVM_EXPECT_BB(stack_begin, <=, stack_ptr, <=, static_cast<const void*>(operands_end));
  LAMAVM_EXPECT_BB(0, <=, count, <=, operands_end - sp);

  return {sp, static_cast<std::size_t>(count)};
}

box stack_pop_arg() {
  static_cast<void>(stack_args(1));
  return interpreter::stack_pop();
}

void stack_push_frame(frame f) {
  LAMAVM_EXPECT_B(frame::size, <=, sp - stack.data());
  interpreter::stack_push_frame(f);
}

void stack_init_locals(op::word_t* entry) {
  const auto [n_args, n_locals] = entry[1].as_i32;
  const auto* begin = frame_begin();
  LAMAVM_EXPECT_B(static_cast<const void*>(sp), ==, static_cast<const void*>(begin));
  LAMAVM_EXPECT_B(n_args, ==, stack_n_args());
  LAMAVM_EXPECT_BB(0, <=, n_locals, <=, begin - stack.data());

  for (std::int32_t i = 0; i < n_locals; ++i) {
    cold::stack_push(box(std::int64_t{0}));
  }

  if (unbox<box*>(fp[frame::offset_fp]) == nullptr) {
    return_pc()[-2].as_target = entry;
  }
}

std::int32_t stack_n_args() {
  return return_pc()[-1].as_call.count;
}

std::int64_t stack_n_locals() {
  static_cast<void>(frame_begin());
  const auto* entry = frame_entry();
  return entry == nullptr ? 0 : entry[1].as_i32.arg1;
}

std::span<box> stack_frame_args() {
  static_cast<void>(frame_begin());
  auto* args = fp + 1;
  const auto n_args = stack_n_args();
  LAMAVM_EXPECT_BB(0, <=, n_args, <=, stack.data() + stack.size() - args);

  return {args, static_cast<std::size_t>(n_args)};
}

box stack_frame_closure() {
  const auto args = stack_frame_args();
  const auto* slot = args.data() + args.size();
  LAMAVM_EXPECT_C(slot != stack.data() + stack.size(), "closure slot is outside the stack");

  const void* saved_sp = unbox<box*>(fp[frame::offset_sp]);
  LAMAVM_EXPECT_C(slot + 1 == saved_sp, "current frame has no closure");

  return *slot;
}

} // namespace lamavm::interpreter::cold
