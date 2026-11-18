#include "specialization.hpp"
#include "handler.hpp"

#include "interpreter/runtime/box.hpp"
#include "interpreter/runtime/state.hpp"
#include "interpreter/std/std.hpp"
#include "op/word.hpp"

#include <bit>
#include <cstdint>

namespace lamavm::interpreter::warm {

namespace {

void specialize_target(op::word_t* ip, op::word_t* target, const uctx* callee) {
  const auto count = ip[2].as_i32.arg1;
  ip[1].as_target = target;
  ip[2].as_call = {.unit_index = static_cast<std::uint32_t>(callee - units.data()), .count = count};
}

} // namespace

void specialize_op_add(op::word_t* ip) {
  ip->as_handler_fn = op_add;
}

void specialize_op_sub(op::word_t* ip) {
  ip->as_handler_fn = op_sub;
}

void specialize_op_mul(op::word_t* ip) {
  ip->as_handler_fn = op_mul;
}

void specialize_op_div(op::word_t* ip) {
  ip->as_handler_fn = op_div;
}

void specialize_op_mod(op::word_t* ip) {
  ip->as_handler_fn = op_mod;
}

void specialize_op_lt(op::word_t* ip) {
  ip->as_handler_fn = op_lt;
}

void specialize_op_le(op::word_t* ip) {
  ip->as_handler_fn = op_le;
}

void specialize_op_gt(op::word_t* ip) {
  ip->as_handler_fn = op_gt;
}

void specialize_op_ge(op::word_t* ip) {
  ip->as_handler_fn = op_ge;
}

void specialize_op_eq(op::word_t* ip) {
  ip->as_handler_fn = op_eq;
}

void specialize_op_ne(op::word_t* ip) {
  ip->as_handler_fn = op_ne;
}

void specialize_op_and(op::word_t* ip) {
  ip->as_handler_fn = op_and;
}

void specialize_op_or(op::word_t* ip) {
  ip->as_handler_fn = op_or;
}

void specialize_op_const(op::word_t* ip, box value) {
  ip[1].as_box_bits = std::bit_cast<std::uint64_t>(value);
  ip->as_handler_fn = op_const;
}

void specialize_op_string(op::word_t* ip, const char* str) {
  ip[1].as_str = str;
  ip->as_handler_fn = op_string;
}

void specialize_op_sexp(op::word_t* ip, box value) {
  ip[1].as_box_bits = std::bit_cast<std::uint64_t>(value);
  ip->as_handler_fn = op_sexp;
}

void specialize_op_sta(op::word_t* ip) {
  ip->as_handler_fn = op_sta;
}

void specialize_op_jmp(op::word_t* ip, op::word_t* target) {
  ip[1].as_target = target;
  ip->as_handler_fn = op_jmp;
}

void specialize_op_end(op::word_t* ip) {
  ip->as_handler_fn = op_end;
}

void specialize_op_drop(op::word_t* ip) {
  ip->as_handler_fn = op_drop;
}

void specialize_op_dup(op::word_t* ip) {
  ip->as_handler_fn = op_dup;
}

void specialize_op_swap(op::word_t* ip) {
  ip->as_handler_fn = op_swap;
}

void specialize_op_elem(op::word_t* ip) {
  ip->as_handler_fn = op_elem;
}

void specialize_op_ld_glo(op::word_t* ip, box& slot) {
  ip[1].as_slot = &slot;
  ip->as_handler_fn = op_ld_glo;
}

void specialize_op_ld_loc(op::word_t* ip, std::int32_t offset) {
  ip[1].as_i32.arg0 = offset;
  ip->as_handler_fn = op_ld_loc;
}

void specialize_op_ld_arg(op::word_t* ip, std::int32_t offset) {
  ip[1].as_i32.arg0 = offset;
  ip->as_handler_fn = op_ld_arg;
}

void specialize_op_ld_clo(op::word_t* ip, std::int32_t frame_offset, std::int32_t capture_offset) {
  ip[1].as_i32 = {.arg0 = frame_offset, .arg1 = capture_offset};
  ip->as_handler_fn = op_ld_clo;
}

void specialize_op_st_glo(op::word_t* ip, box& slot) {
  ip[1].as_slot = &slot;
  ip->as_handler_fn = op_st_glo;
}

void specialize_op_st_loc(op::word_t* ip, std::int32_t offset) {
  ip[1].as_i32.arg0 = offset;
  ip->as_handler_fn = op_st_loc;
}

void specialize_op_st_arg(op::word_t* ip, std::int32_t offset) {
  ip[1].as_i32.arg0 = offset;
  ip->as_handler_fn = op_st_arg;
}

void specialize_op_st_clo(op::word_t* ip, std::int32_t frame_offset, std::int32_t capture_offset) {
  ip[1].as_i32 = {.arg0 = frame_offset, .arg1 = capture_offset};
  ip->as_handler_fn = op_st_clo;
}

void specialize_op_cjmp_z(op::word_t* ip, op::word_t* target) {
  ip[1].as_target = target;
  ip->as_handler_fn = op_cjmp_z;
}

void specialize_op_cjmp_nz(op::word_t* ip, op::word_t* target) {
  ip[1].as_target = target;
  ip->as_handler_fn = op_cjmp_nz;
}

void specialize_op_begin(op::word_t* ip) {
  ip->as_handler_fn = ip[1].as_i32.arg1 == 0 ? op_begin_no_locals : op_begin;
}

void specialize_op_begin_closure(op::word_t* ip) {
  ip->as_handler_fn = ip[1].as_i32.arg1 == 0 ? op_begin_closure_no_locals : op_begin_closure;
}

void specialize_op_closure(op::word_t* ip, op::word_t* target, const uctx* callee) {
  specialize_target(ip, target, callee);
  ip->as_handler_fn = op_closure;
}

void specialize_op_callc(op::word_t* ip) {
  const auto count = ip[1].as_i32.arg0;
  ip[1].as_call = {.unit_index = op::call_t::indirect, .count = count};
  ip->as_handler_fn = op_callc;
}

void specialize_op_call(
  op::word_t* ip,
  op::word_t* target,
  const uctx* callee,
  const uctx* caller) {
  specialize_target(ip, target, callee);
  ip->as_handler_fn = callee == caller ? op_call_local : op_call;
}

void specialize_op_tag(op::word_t* ip, box hash, box count) {
  ip[1].as_box_bits = std::bit_cast<std::uint64_t>(hash);
  ip[2].as_box_bits = std::bit_cast<std::uint64_t>(count);
  ip->as_handler_fn = op_tag;
}

void specialize_op_array(op::word_t* ip, box value) {
  ip[1].as_box_bits = std::bit_cast<std::uint64_t>(value);
  ip->as_handler_fn = op_array;
}

void specialize_op_line(op::word_t* ip) {
  ip->as_handler_fn = op_line;
}

void specialize_op_patt_str_cmp(op::word_t* ip) {
  ip->as_handler_fn = op_patt_str_cmp;
}

void specialize_op_patt_string(op::word_t* ip) {
  ip->as_handler_fn = op_patt_string;
}

void specialize_op_patt_array(op::word_t* ip) {
  ip->as_handler_fn = op_patt_array;
}

void specialize_op_patt_sexp(op::word_t* ip) {
  ip->as_handler_fn = op_patt_sexp;
}

void specialize_op_patt_boxed(op::word_t* ip) {
  ip->as_handler_fn = op_patt_boxed;
}

void specialize_op_patt_unboxed(op::word_t* ip) {
  ip->as_handler_fn = op_patt_unboxed;
}

void specialize_op_patt_closure(op::word_t* ip) {
  ip->as_handler_fn = op_patt_closure;
}

void specialize_op_barray(op::word_t* ip) {
  ip->as_handler_fn = op_barray;
}

void specialize_op_call_std(op::word_t* ip, std_fn_some_t fn) {
  ip[1].as_std_fn = fn;
  ip->as_handler_fn = op_call_std;
}

} // namespace lamavm::interpreter::warm
