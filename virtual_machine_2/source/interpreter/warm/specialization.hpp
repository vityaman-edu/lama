#pragma once

#include "interpreter/runtime/box.hpp"
#include "interpreter/std/std.hpp"
#include "op/word.hpp"

#include <cstdint>

namespace lamavm::interpreter {
struct uctx;
} // namespace lamavm::interpreter

namespace lamavm::interpreter::warm {

void specialize_op_add(op::word_t* ip);

void specialize_op_sub(op::word_t* ip);

void specialize_op_mul(op::word_t* ip);

void specialize_op_div(op::word_t* ip);

void specialize_op_mod(op::word_t* ip);

void specialize_op_lt(op::word_t* ip);

void specialize_op_le(op::word_t* ip);

void specialize_op_gt(op::word_t* ip);

void specialize_op_ge(op::word_t* ip);

void specialize_op_eq(op::word_t* ip);

void specialize_op_ne(op::word_t* ip);

void specialize_op_and(op::word_t* ip);

void specialize_op_or(op::word_t* ip);

void specialize_op_const(op::word_t* ip, box value);

void specialize_op_string(op::word_t* ip, const char* str);

void specialize_op_sexp(op::word_t* ip, box value);

void specialize_op_sta(op::word_t* ip);

void specialize_op_jmp(op::word_t* ip, op::word_t* target);

void specialize_op_end(op::word_t* ip);

void specialize_op_drop(op::word_t* ip);

void specialize_op_dup(op::word_t* ip);

void specialize_op_swap(op::word_t* ip);

void specialize_op_elem(op::word_t* ip);

void specialize_op_ld_glo(op::word_t* ip, box& slot);

void specialize_op_ld_loc(op::word_t* ip, std::int32_t offset);

void specialize_op_ld_arg(op::word_t* ip, std::int32_t offset);

void specialize_op_ld_clo(op::word_t* ip, std::int32_t frame_offset, std::int32_t capture_offset);

void specialize_op_st_glo(op::word_t* ip, box& slot);

void specialize_op_st_loc(op::word_t* ip, std::int32_t offset);

void specialize_op_st_arg(op::word_t* ip, std::int32_t offset);

void specialize_op_st_clo(op::word_t* ip, std::int32_t frame_offset, std::int32_t capture_offset);

void specialize_op_cjmp_z(op::word_t* ip, op::word_t* target);

void specialize_op_cjmp_nz(op::word_t* ip, op::word_t* target);

void specialize_op_begin(op::word_t* ip);

void specialize_op_begin_closure(op::word_t* ip);

void specialize_op_closure(op::word_t* ip, op::word_t* target, const uctx* callee);

void specialize_op_callc(op::word_t* ip);

void specialize_op_call(op::word_t* ip, op::word_t* target, const uctx* callee, const uctx* caller);

void specialize_op_tag(op::word_t* ip, box hash, box count);

void specialize_op_array(op::word_t* ip, box value);

void specialize_op_line(op::word_t* ip);

void specialize_op_patt_str_cmp(op::word_t* ip);

void specialize_op_patt_string(op::word_t* ip);

void specialize_op_patt_array(op::word_t* ip);

void specialize_op_patt_sexp(op::word_t* ip);

void specialize_op_patt_boxed(op::word_t* ip);

void specialize_op_patt_unboxed(op::word_t* ip);

void specialize_op_patt_closure(op::word_t* ip);

void specialize_op_barray(op::word_t* ip);

void specialize_op_call_std(op::word_t* ip, std_fn_some_t fn);

} // namespace lamavm::interpreter::warm
