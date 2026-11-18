#include "handler.hpp"

#include "core/log.hpp"
#include "interpreter/runtime/box.hpp"
#include "interpreter/runtime/state.hpp"
#include "interpreter/std/std.hpp"
#include "op/word.hpp"

#include <algorithm>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <span>
#include <utility>

namespace lamavm::interpreter::warm {

namespace {

void binary(auto f) {
  sp[1] = f(sp[1], sp[0]);
  ++sp;
}

box boxed_word(const op::word_t& word) {
  return std::bit_cast<box>(word.as_box_bits);
}

box& capture_slot() {
  const auto [frame_offset, capture_offset] = pc[1].as_i32;
  auto* closure = unbox<box*>(fp[frame_offset]);
  return closure[capture_offset];
}

void init_locals(std::int32_t n_locals) {
  sp -= n_locals;
  std::fill_n(sp, n_locals, box(std::int64_t{0}));
}

void push_frame(op::word_t* continuation, box* caller_sp) {
  stack_push_frame({
    .fp = fp,
    .pc = continuation,
    .sp = caller_sp,
    .up = up,
  });
}

box capture_value(op::capture_t capture) {
  if (capture.base == op::capture_t::global_base) {
    return stack[capture.offset];
  }

  if (capture.base == op::capture_t::frame_base) {
    return fp[capture.offset];
  }

  const auto* closure = unbox<box*>(fp[capture.base]);
  return closure[capture.offset];
}

} // namespace

void op_add() {
  binary(boxd::Ls__Infix_43);
  LAMAVM_DISPATCH();
}

void op_sub() {
  binary(boxd::Ls__Infix_45);
  LAMAVM_DISPATCH();
}

void op_mul() {
  binary(boxd::Ls__Infix_42);
  LAMAVM_DISPATCH();
}

void op_div() {
  binary(boxd::Ls__Infix_47);
  LAMAVM_DISPATCH();
}

void op_mod() {
  binary(boxd::Ls__Infix_37);
  LAMAVM_DISPATCH();
}

void op_lt() {
  binary(boxd::Ls__Infix_60);
  LAMAVM_DISPATCH();
}

void op_le() {
  binary(boxd::Ls__Infix_6061);
  LAMAVM_DISPATCH();
}

void op_gt() {
  binary(boxd::Ls__Infix_62);
  LAMAVM_DISPATCH();
}

void op_ge() {
  binary(boxd::Ls__Infix_6261);
  LAMAVM_DISPATCH();
}

void op_eq() {
  binary(boxd::Ls__Infix_6161);
  LAMAVM_DISPATCH();
}

void op_ne() {
  binary(boxd::Ls__Infix_3361);
  LAMAVM_DISPATCH();
}

void op_and() {
  binary(boxd::Ls__Infix_3838);
  LAMAVM_DISPATCH();
}

void op_or() {
  binary(boxd::Ls__Infix_3333);
  LAMAVM_DISPATCH();
}

void op_const() {
  const auto value = boxed_word(pc[1]);
  stack_push(value);
  ++pc;
  LAMAVM_DISPATCH();
}

void op_string() {
  const char* literal = pc[1].as_str;
  const box argument(static_cast<const void*>(&literal));
  const auto string = gc_call([&] { return boxd::Bstring(argument); });
  stack_push(string);
  ++pc;
  LAMAVM_DISPATCH();
}

void op_sexp() {
  const auto hash = boxed_word(pc[1]);
  const auto n_fields = pc[2].as_i32.arg0;
  const auto count = std::int64_t{n_fields} + 1;

  stack_push(hash);
  std::reverse(sp, sp + count);
  const auto result = gc_call([&] { return boxd::Bsexp(box(sp), box(count)); });
  sp += count;
  stack_push(result);
  pc += 2;
  LAMAVM_DISPATCH();
}

void op_sta() {
  sp[2] = boxd::Bsta(sp[2], sp[1], sp[0]);
  sp += 2;
  LAMAVM_DISPATCH();
}

void op_jmp() {
  pc = pc[1].as_target;
  LAMAVM_DISPATCH_JUMP();
}

void op_end() {
  const auto result = *sp;
  stack_pop_frame();
  stack_push(result);
  LAMAVM_DISPATCH_JUMP();
}

void op_drop() {
  ++sp;
  LAMAVM_DISPATCH();
}

void op_dup() {
  stack_push(*sp);
  LAMAVM_DISPATCH();
}

void op_swap() {
  std::swap(sp[0], sp[1]);
  LAMAVM_DISPATCH();
}

void op_elem() {
  binary(boxd::Belem);
  LAMAVM_DISPATCH();
}

void op_ld_glo() {
  const auto* slot = pc[1].as_slot;
  stack_push(*slot);
  ++pc;
  LAMAVM_DISPATCH();
}

void op_ld_arg() {
  const auto offset = pc[1].as_i32.arg0;
  const auto value = fp[offset];
  stack_push(value);
  ++pc;
  LAMAVM_DISPATCH();
}

void op_ld_loc() {
  const auto offset = pc[1].as_i32.arg0;
  const auto value = fp[offset];
  stack_push(value);
  ++pc;
  LAMAVM_DISPATCH();
}

void op_ld_clo() {
  stack_push(capture_slot());
  ++pc;
  LAMAVM_DISPATCH();
}

void op_st_glo() {
  auto* slot = pc[1].as_slot;
  *slot = *sp;
  ++pc;
  LAMAVM_DISPATCH();
}

void op_st_arg() {
  const auto offset = pc[1].as_i32.arg0;
  fp[offset] = *sp;
  ++pc;
  LAMAVM_DISPATCH();
}

void op_st_loc() {
  const auto offset = pc[1].as_i32.arg0;
  fp[offset] = *sp;
  ++pc;
  LAMAVM_DISPATCH();
}

void op_st_clo() {
  capture_slot() = *sp;
  ++pc;
  LAMAVM_DISPATCH();
}

void op_cjmp_z() {
  auto* target = pc[1].as_target;
  const auto condition = unbox<std::int64_t>(stack_pop());
  pc = condition == 0 ? target : pc + 2;
  LAMAVM_DISPATCH_JUMP();
}

void op_cjmp_nz() {
  auto* target = pc[1].as_target;
  const auto condition = unbox<std::int64_t>(stack_pop());
  pc = condition != 0 ? target : pc + 2;
  LAMAVM_DISPATCH_JUMP();
}

void op_begin() {
  const auto n_locals = pc[1].as_i32.arg1;
  init_locals(n_locals);
  ++pc;
  LAMAVM_DISPATCH();
}

void op_begin_no_locals() {
  ++pc;
  LAMAVM_DISPATCH();
}

void op_begin_closure() {
  const auto n_locals = pc[1].as_i32.arg1;
  init_locals(n_locals);
  pc += 2;
  LAMAVM_DISPATCH();
}

void op_begin_closure_no_locals() {
  pc += 2;
  LAMAVM_DISPATCH();
}

void op_closure() {
  const auto* target = pc[1].as_target;
  const auto [unit_index, count] = pc[2].as_call;
  const auto* callee = &units[unit_index];
  const auto* captures = pc + 3;
  pc += 3 + count;

  for (auto i = count; i > 0; --i) {
    const auto descriptor = captures[i - 1].as_capture;
    stack_push(capture_value(descriptor));
  }

  stack_push(box(callee));
  const auto target_offset = target - callee->text.data();
  stack_push(box(target_offset));
  const auto closure = gc_call([&] { return boxd::Bclosure(box(sp), box(1 + count)); });
  sp += 2 + count;
  stack_push(closure);
  LAMAVM_DISPATCH_JUMP();
}

void op_callc() {
  const auto n_args = pc[1].as_call.count;
  const auto* closure = unbox<box*>(sp[n_args]);
  const auto target_offset = unbox<std::int32_t>(closure[0]);
  const auto* callee = unbox<const uctx*>(closure[1]);
  auto* target = callee->text.data() + target_offset;

  tracef("warm callc {} argc={}", callee->name, n_args);
  push_frame(pc + 2, sp + n_args + 1);
  up = callee;
  pc = target;
  LAMAVM_DISPATCH_JUMP();
}

void op_call() {
  const auto [unit_index, n_args] = pc[2].as_call;
  auto* target = pc[1].as_target;
  const auto* callee = &units[unit_index];

  tracef("warm call {} -> {} argc={}", up->name, callee->name, n_args);
  push_frame(pc + 3, sp + n_args);
  up = callee;
  pc = target;
  LAMAVM_DISPATCH_JUMP();
}

void op_call_local() {
  const auto n_args = pc[2].as_call.count;
  auto* target = pc[1].as_target;

  tracef("warm local call {} argc={}", up->name, n_args);
  push_frame(pc + 3, sp + n_args);
  pc = target;
  LAMAVM_DISPATCH_JUMP();
}

void op_call_std() {
  const auto n_args = pc[2].as_i32.arg1;
  const auto fn = pc[1].as_std_fn;
  const std::span<box> args(sp, static_cast<std::size_t>(n_args));

  tracef("warm Std call argc={}", n_args);
  const auto result = gc_call([&] { return fn(args); });
  sp += n_args;
  stack_push(result);
  pc += 2;
  LAMAVM_DISPATCH();
}

void op_tag() {
  const auto hash = boxed_word(pc[1]);
  const auto count = boxed_word(pc[2]);
  *sp = boxd::Btag(*sp, hash, count);
  pc += 2;
  LAMAVM_DISPATCH();
}

void op_array() {
  const auto count = boxed_word(pc[1]);
  *sp = boxd::Barray_patt(*sp, count);
  ++pc;
  LAMAVM_DISPATCH();
}

void op_line() {
  ++pc;
  LAMAVM_DISPATCH();
}

void op_patt_str_cmp() {
  binary(boxd::Bstring_patt);
  LAMAVM_DISPATCH();
}

void op_patt_string() {
  *sp = boxd::Bstring_tag_patt(*sp);
  LAMAVM_DISPATCH();
}

void op_patt_array() {
  *sp = boxd::Barray_tag_patt(*sp);
  LAMAVM_DISPATCH();
}

void op_patt_sexp() {
  *sp = boxd::Bsexp_tag_patt(*sp);
  LAMAVM_DISPATCH();
}

void op_patt_boxed() {
  *sp = boxd::Bboxed_patt(*sp);
  LAMAVM_DISPATCH();
}

void op_patt_unboxed() {
  *sp = boxd::Bunboxed_patt(*sp);
  LAMAVM_DISPATCH();
}

void op_patt_closure() {
  *sp = boxd::Bclosure_tag_patt(*sp);
  LAMAVM_DISPATCH();
}

void op_barray() {
  const auto count = pc[1].as_i32.arg0;
  std::reverse(sp, sp + count);
  const auto result = gc_call([&] { return boxd::Barray(box(sp), box(count)); });
  sp += count;
  stack_push(result);
  ++pc;
  LAMAVM_DISPATCH();
}

} // namespace lamavm::interpreter::warm
