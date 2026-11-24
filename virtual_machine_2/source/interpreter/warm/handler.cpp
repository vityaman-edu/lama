#include "handler.hpp"
#include "layout.hpp"

#include "core/assert.hpp"
#include "core/log.hpp"
#include "interpreter/runtime/box.hpp"
#include "interpreter/runtime/closure.hpp"
#include "interpreter/runtime/state.hpp"
#include "interpreter/std/std.hpp"
#include "op/word.hpp"

#include <algorithm>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <ranges>
#include <span>
#include <tuple>
#include <utility>

namespace lamavm::interpreter::warm {

namespace {

void binary(auto f) {
  sp[1] = f(sp[1], sp[0]);
  ++sp;
}

box& capture_slot(std::int32_t frame_offset, std::int32_t capture_offset) {
  const auto closure = closure_payload<soft>(fp[frame_offset]);
  return closure[capture_offset];
}

void init_locals(std::int32_t n_locals) {
  sp -= n_locals;
  std::fill_n(static_cast<box*>(sp), n_locals, box(std::int64_t{0}));
}

void push_frame(op::word_t* continuation, box* caller_sp, std::int32_t n_args) {
  stack_push_frame({
    .n_args = n_args,
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

  const auto closure = closure_payload<soft>(fp[capture.base]);
  return closure[capture.offset];
}

} // namespace

void op_add() {
  std::ignore = op_add_t::reinterpret(pc);
  binary(boxd::Ls__Infix_43);
  LAMAVM_DISPATCH(op_add_t);
}

void op_sub() {
  std::ignore = op_sub_t::reinterpret(pc);
  binary(boxd::Ls__Infix_45);
  LAMAVM_DISPATCH(op_sub_t);
}

void op_mul() {
  std::ignore = op_mul_t::reinterpret(pc);
  binary(boxd::Ls__Infix_42);
  LAMAVM_DISPATCH(op_mul_t);
}

void op_div() {
  std::ignore = op_div_t::reinterpret(pc);
  binary(boxd::Ls__Infix_47);
  LAMAVM_DISPATCH(op_div_t);
}

void op_mod() {
  std::ignore = op_mod_t::reinterpret(pc);
  binary(boxd::Ls__Infix_37);
  LAMAVM_DISPATCH(op_mod_t);
}

void op_lt() {
  std::ignore = op_lt_t::reinterpret(pc);
  binary(boxd::Ls__Infix_60);
  LAMAVM_DISPATCH(op_lt_t);
}

void op_le() {
  std::ignore = op_le_t::reinterpret(pc);
  binary(boxd::Ls__Infix_6061);
  LAMAVM_DISPATCH(op_le_t);
}

void op_gt() {
  std::ignore = op_gt_t::reinterpret(pc);
  binary(boxd::Ls__Infix_62);
  LAMAVM_DISPATCH(op_gt_t);
}

void op_ge() {
  std::ignore = op_ge_t::reinterpret(pc);
  binary(boxd::Ls__Infix_6261);
  LAMAVM_DISPATCH(op_ge_t);
}

void op_eq() {
  std::ignore = op_eq_t::reinterpret(pc);
  binary(boxd::Ls__Infix_6161);
  LAMAVM_DISPATCH(op_eq_t);
}

void op_ne() {
  std::ignore = op_ne_t::reinterpret(pc);
  binary(boxd::Ls__Infix_3361);
  LAMAVM_DISPATCH(op_ne_t);
}

void op_and() {
  std::ignore = op_and_t::reinterpret(pc);
  binary(boxd::Ls__Infix_3838);
  LAMAVM_DISPATCH(op_and_t);
}

void op_or() {
  std::ignore = op_or_t::reinterpret(pc);
  binary(boxd::Ls__Infix_3333);
  LAMAVM_DISPATCH(op_or_t);
}

void op_const() {
  const auto* args = op_const_t::reinterpret(pc);
  const auto value = std::bit_cast<box>(args->value_bits);
  stack_push(value);
  LAMAVM_DISPATCH(op_const_t);
}

void op_string() {
  const auto* args = op_string_t::reinterpret(pc);
  const char* literal = args->literal;
  const box argument(static_cast<const void*>(&literal));
  const auto string = boxd::Bstring(argument);
  stack_push(string);
  LAMAVM_DISPATCH(op_string_t);
}

void op_sexp() {
  const auto* args = op_sexp_t::reinterpret(pc);
  const auto hash = std::bit_cast<box>(args->hash_bits);
  const auto n_fields = args->n_fields;
  const auto count = std::int64_t{n_fields} + 1;

  stack_push(hash);
  std::reverse(static_cast<box*>(sp), sp + count);
  const auto result = boxd::Bsexp(box(static_cast<box*>(sp)), box(count));
  sp += count;
  stack_push(result);
  LAMAVM_DISPATCH(op_sexp_t);
}

void op_sta() {
  std::ignore = op_sta_t::reinterpret(pc);
  sp[2] = boxd::Bsta(sp[2], sp[1], sp[0]);
  sp += 2;
  LAMAVM_DISPATCH(op_sta_t);
}

void op_jmp() {
  const auto* args = op_jmp_t::reinterpret(pc);
  pc = args->target;
  LAMAVM_DISPATCH_JUMP();
}

void op_end() {
  std::ignore = op_end_t::reinterpret(pc);
  const auto result = *sp;
  stack_pop_frame();
  stack_push(result);
  LAMAVM_DISPATCH_JUMP();
}

void op_drop() {
  std::ignore = op_drop_t::reinterpret(pc);
  ++sp;
  LAMAVM_DISPATCH(op_drop_t);
}

void op_dup() {
  std::ignore = op_dup_t::reinterpret(pc);
  stack_push(*sp);
  LAMAVM_DISPATCH(op_dup_t);
}

void op_swap() {
  std::ignore = op_swap_t::reinterpret(pc);
  std::swap(sp[0], sp[1]);
  LAMAVM_DISPATCH(op_swap_t);
}

void op_elem() {
  std::ignore = op_elem_t::reinterpret(pc);
  binary(boxd::Belem);
  LAMAVM_DISPATCH(op_elem_t);
}

void op_ld_glo() {
  const auto* args = op_ld_glo_t::reinterpret(pc);
  const auto* slot = args->slot;
  stack_push(*slot);
  LAMAVM_DISPATCH(op_ld_glo_t);
}

void op_ld_arg() {
  const auto* args = op_ld_arg_t::reinterpret(pc);
  const auto offset = args->offset;
  const auto value = fp[offset];
  stack_push(value);
  LAMAVM_DISPATCH(op_ld_arg_t);
}

void op_ld_loc() {
  const auto* args = op_ld_loc_t::reinterpret(pc);
  const auto offset = args->offset;
  const auto value = fp[offset];
  stack_push(value);
  LAMAVM_DISPATCH(op_ld_loc_t);
}

void op_ld_clo() {
  const auto* args = op_ld_clo_t::reinterpret(pc);
  stack_push(capture_slot(args->frame_offset, args->capture_offset));
  LAMAVM_DISPATCH(op_ld_clo_t);
}

void op_st_glo() {
  const auto* args = op_st_glo_t::reinterpret(pc);
  auto* slot = args->slot;
  *slot = *sp;
  LAMAVM_DISPATCH(op_st_glo_t);
}

void op_st_arg() {
  const auto* args = op_st_arg_t::reinterpret(pc);
  const auto offset = args->offset;
  fp[offset] = *sp;
  LAMAVM_DISPATCH(op_st_arg_t);
}

void op_st_loc() {
  const auto* args = op_st_loc_t::reinterpret(pc);
  const auto offset = args->offset;
  fp[offset] = *sp;
  LAMAVM_DISPATCH(op_st_loc_t);
}

void op_st_clo() {
  const auto* args = op_st_clo_t::reinterpret(pc);
  capture_slot(args->frame_offset, args->capture_offset) = *sp;
  LAMAVM_DISPATCH(op_st_clo_t);
}

void op_cjmp_z() {
  const auto* args = op_cjmp_z_t::reinterpret(pc);
  auto* target = args->target;
  const auto condition = unbox<std::int64_t>(stack_pop());
  pc = condition == 0 ? target : pc + word_count<op_cjmp_z_t>();
  LAMAVM_DISPATCH_JUMP();
}

void op_cjmp_nz() {
  const auto* args = op_cjmp_nz_t::reinterpret(pc);
  auto* target = args->target;
  const auto condition = unbox<std::int64_t>(stack_pop());
  pc = condition != 0 ? target : pc + word_count<op_cjmp_nz_t>();
  LAMAVM_DISPATCH_JUMP();
}

void op_begin() {
  const auto* args = op_begin_t::reinterpret(pc);
  const auto n_locals = args->n_locals;
  init_locals(n_locals);
  LAMAVM_DISPATCH(op_begin_t);
}

void op_begin_no_locals() {
  std::ignore = op_begin_no_locals_t::reinterpret(pc);
  LAMAVM_DISPATCH(op_begin_no_locals_t);
}

void op_begin_closure() {
  const auto* args = op_begin_closure_t::reinterpret(pc);
  const auto n_locals = args->n_locals;
  init_locals(n_locals);
  LAMAVM_DISPATCH(op_begin_closure_t);
}

void op_begin_closure_no_locals() {
  std::ignore = op_begin_closure_no_locals_t::reinterpret(pc);
  LAMAVM_DISPATCH(op_begin_closure_no_locals_t);
}

void op_closure() {
  const auto args = op_closure_view_t::reinterpret(pc);
  const auto& [header, captures] = args;
  const auto* callee = &units[header->unit_index];
  const auto count = header->n_captured;

  for (const auto& capture : std::views::reverse(captures)) {
    stack_push(capture_value(capture));
  }

  stack_push(box(callee));
  const auto target_offset = header->target - callee->text.data();
  stack_push(box(target_offset));
  const auto closure = boxd::Bclosure(box(static_cast<box*>(sp)), box(1 + count));
  sp += 2 + count;
  stack_push(closure);
  pc += word_count(args);
  LAMAVM_DISPATCH_JUMP();
}

void op_callc() {
  const auto* args = op_callc_t::reinterpret(pc);
  const auto n_args = args->n_args;
  const auto closure = closure_payload<soft>(sp[n_args]);
  const auto target_offset = unbox<std::int32_t>(closure[0]);
  const auto* callee = unbox<const uctx*>(closure[1]);
  auto* target = callee->text.data() + target_offset;

  tracef("warm callc {} argc={}", callee->name, n_args);
  push_frame(pc + word_count<op_callc_t>(), sp + n_args + 1, n_args);
  up = callee;
  pc = target;
  LAMAVM_DISPATCH_JUMP();
}

void op_call() {
  const auto* args = op_call_t::reinterpret(pc);
  const auto n_args = args->n_args;
  const auto* callee = &units[args->unit_index];
  auto* target = args->target;

  tracef("warm call {} -> {} argc={}", up->name, callee->name, n_args);
  push_frame(pc + word_count<op_call_t>(), sp + n_args, n_args);
  up = callee;
  pc = target;
  LAMAVM_DISPATCH_JUMP();
}

void op_call_local() {
  const auto* args = op_call_local_t::reinterpret(pc);
  const auto n_args = args->n_args;
  auto* target = args->target;

  tracef("warm local call {} argc={}", up->name, n_args);
  push_frame(pc + word_count<op_call_local_t>(), sp + n_args, n_args);
  pc = target;
  LAMAVM_DISPATCH_JUMP();
}

void op_call_std() {
  const auto* args = op_call_std_t::reinterpret(pc);
  const auto n_args = args->n_args;
  const auto fn = args->fn;
  const std::span<box> values(static_cast<box*>(sp), static_cast<std::size_t>(n_args));

  tracef("warm Std call argc={}", n_args);
  const auto result = fn(values);
  sp += n_args;
  stack_push(result);
  LAMAVM_DISPATCH(op_call_std_t);
}

void op_tag() {
  const auto* args = op_tag_t::reinterpret(pc);
  const auto hash = std::bit_cast<box>(args->hash_bits);
  const auto count = std::bit_cast<box>(args->n_fields_bits);
  *sp = boxd::Btag(*sp, hash, count);
  LAMAVM_DISPATCH(op_tag_t);
}

void op_array() {
  const auto* args = op_array_t::reinterpret(pc);
  const auto count = std::bit_cast<box>(args->n_elements_bits);
  *sp = boxd::Barray_patt(*sp, count);
  LAMAVM_DISPATCH(op_array_t);
}

void op_line() {
  std::ignore = op_line_t::reinterpret(pc);
  LAMAVM_DISPATCH(op_line_t);
}

void op_patt_str_cmp() {
  std::ignore = op_patt_str_cmp_t::reinterpret(pc);
  binary(boxd::Bstring_patt);
  LAMAVM_DISPATCH(op_patt_str_cmp_t);
}

void op_patt_string() {
  std::ignore = op_patt_string_t::reinterpret(pc);
  *sp = boxd::Bstring_tag_patt(*sp);
  LAMAVM_DISPATCH(op_patt_string_t);
}

void op_patt_array() {
  std::ignore = op_patt_array_t::reinterpret(pc);
  *sp = boxd::Barray_tag_patt(*sp);
  LAMAVM_DISPATCH(op_patt_array_t);
}

void op_patt_sexp() {
  std::ignore = op_patt_sexp_t::reinterpret(pc);
  *sp = boxd::Bsexp_tag_patt(*sp);
  LAMAVM_DISPATCH(op_patt_sexp_t);
}

void op_patt_boxed() {
  std::ignore = op_patt_boxed_t::reinterpret(pc);
  *sp = boxd::Bboxed_patt(*sp);
  LAMAVM_DISPATCH(op_patt_boxed_t);
}

void op_patt_unboxed() {
  std::ignore = op_patt_unboxed_t::reinterpret(pc);
  *sp = boxd::Bunboxed_patt(*sp);
  LAMAVM_DISPATCH(op_patt_unboxed_t);
}

void op_patt_closure() {
  std::ignore = op_patt_closure_t::reinterpret(pc);
  *sp = boxd::Bclosure_tag_patt(*sp);
  LAMAVM_DISPATCH(op_patt_closure_t);
}

void op_barray() {
  const auto* args = op_barray_t::reinterpret(pc);
  const auto count = args->n_elements;
  std::reverse(static_cast<box*>(sp), sp + count);
  const auto result = boxd::Barray(box(static_cast<box*>(sp)), box(count));
  sp += count;
  stack_push(result);
  LAMAVM_DISPATCH(op_barray_t);
}

} // namespace lamavm::interpreter::warm
