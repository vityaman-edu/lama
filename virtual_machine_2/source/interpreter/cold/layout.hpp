#pragma once

#include "core/c.hpp"
#include "op/handler.hpp"
#include "op/word.hpp"

#include <cstddef>
#include <cstdint>
#include <span>

namespace lamavm::interpreter::cold {

struct op_add_t {
  op::handler_fn_t handler;

  [[nodiscard]] static op_add_t* reinterpret(op::word_t* w);
};

struct op_sub_t {
  op::handler_fn_t handler;

  [[nodiscard]] static op_sub_t* reinterpret(op::word_t* w);
};

struct op_mul_t {
  op::handler_fn_t handler;

  [[nodiscard]] static op_mul_t* reinterpret(op::word_t* w);
};

struct op_div_t {
  op::handler_fn_t handler;

  [[nodiscard]] static op_div_t* reinterpret(op::word_t* w);
};

struct op_mod_t {
  op::handler_fn_t handler;

  [[nodiscard]] static op_mod_t* reinterpret(op::word_t* w);
};

struct op_lt_t {
  op::handler_fn_t handler;

  [[nodiscard]] static op_lt_t* reinterpret(op::word_t* w);
};

struct op_le_t {
  op::handler_fn_t handler;

  [[nodiscard]] static op_le_t* reinterpret(op::word_t* w);
};

struct op_gt_t {
  op::handler_fn_t handler;

  [[nodiscard]] static op_gt_t* reinterpret(op::word_t* w);
};

struct op_ge_t {
  op::handler_fn_t handler;

  [[nodiscard]] static op_ge_t* reinterpret(op::word_t* w);
};

struct op_eq_t {
  op::handler_fn_t handler;

  [[nodiscard]] static op_eq_t* reinterpret(op::word_t* w);
};

struct op_ne_t {
  op::handler_fn_t handler;

  [[nodiscard]] static op_ne_t* reinterpret(op::word_t* w);
};

struct op_and_t {
  op::handler_fn_t handler;

  [[nodiscard]] static op_and_t* reinterpret(op::word_t* w);
};

struct op_or_t {
  op::handler_fn_t handler;

  [[nodiscard]] static op_or_t* reinterpret(op::word_t* w);
};

struct op_const_t {
  op::handler_fn_t handler;
  std::int32_t value;
  std::int32_t reserved_1;

  [[nodiscard]] static op_const_t* reinterpret(op::word_t* w);
};

struct op_string_t {
  op::handler_fn_t handler;
  std::int32_t string_offset;
  std::int32_t reserved_1;

  [[nodiscard]] static op_string_t* reinterpret(op::word_t* w);
};

struct op_sexp_t {
  op::handler_fn_t handler;
  std::int32_t tag_offset;
  std::int32_t reserved_1;
  std::int32_t n_fields;
  std::int32_t reserved_2;

  [[nodiscard]] static op_sexp_t* reinterpret(op::word_t* w);
};

struct op_sta_t {
  op::handler_fn_t handler;

  [[nodiscard]] static op_sta_t* reinterpret(op::word_t* w);
};

struct op_jmp_t {
  op::handler_fn_t handler;
  std::int32_t target;
  std::int32_t reserved_1;

  [[nodiscard]] static op_jmp_t* reinterpret(op::word_t* w);
};

struct op_end_t {
  op::handler_fn_t handler;

  [[nodiscard]] static op_end_t* reinterpret(op::word_t* w);
};

struct op_drop_t {
  op::handler_fn_t handler;

  [[nodiscard]] static op_drop_t* reinterpret(op::word_t* w);
};

struct op_dup_t {
  op::handler_fn_t handler;

  [[nodiscard]] static op_dup_t* reinterpret(op::word_t* w);
};

struct op_swap_t {
  op::handler_fn_t handler;

  [[nodiscard]] static op_swap_t* reinterpret(op::word_t* w);
};

struct op_elem_t {
  op::handler_fn_t handler;

  [[nodiscard]] static op_elem_t* reinterpret(op::word_t* w);
};

struct op_ld_glo_t {
  op::handler_fn_t handler;
  std::int32_t index;
  std::int32_t reserved_1;

  [[nodiscard]] static op_ld_glo_t* reinterpret(op::word_t* w);
};

struct op_ld_loc_t {
  op::handler_fn_t handler;
  std::int32_t index;
  std::int32_t reserved_1;

  [[nodiscard]] static op_ld_loc_t* reinterpret(op::word_t* w);
};

struct op_ld_arg_t {
  op::handler_fn_t handler;
  std::int32_t index;
  std::int32_t reserved_1;

  [[nodiscard]] static op_ld_arg_t* reinterpret(op::word_t* w);
};

struct op_ld_clo_t {
  op::handler_fn_t handler;
  std::int32_t index;
  std::int32_t reserved_1;

  [[nodiscard]] static op_ld_clo_t* reinterpret(op::word_t* w);
};

struct op_st_glo_t {
  op::handler_fn_t handler;
  std::int32_t index;
  std::int32_t reserved_1;

  [[nodiscard]] static op_st_glo_t* reinterpret(op::word_t* w);
};

struct op_st_loc_t {
  op::handler_fn_t handler;
  std::int32_t index;
  std::int32_t reserved_1;

  [[nodiscard]] static op_st_loc_t* reinterpret(op::word_t* w);
};

struct op_st_arg_t {
  op::handler_fn_t handler;
  std::int32_t index;
  std::int32_t reserved_1;

  [[nodiscard]] static op_st_arg_t* reinterpret(op::word_t* w);
};

struct op_st_clo_t {
  op::handler_fn_t handler;
  std::int32_t index;
  std::int32_t reserved_1;

  [[nodiscard]] static op_st_clo_t* reinterpret(op::word_t* w);
};

struct op_cjmp_z_t {
  op::handler_fn_t handler;
  std::int32_t target;
  std::int32_t reserved_1;

  [[nodiscard]] static op_cjmp_z_t* reinterpret(op::word_t* w);
};

struct op_cjmp_nz_t {
  op::handler_fn_t handler;
  std::int32_t target;
  std::int32_t reserved_1;

  [[nodiscard]] static op_cjmp_nz_t* reinterpret(op::word_t* w);
};

struct op_begin_t {
  op::handler_fn_t handler;
  std::int32_t n_args;
  std::int32_t n_locals;

  [[nodiscard]] static op_begin_t* reinterpret(op::word_t* w);
};

struct op_begin_closure_t {
  op::handler_fn_t handler;
  std::int32_t n_args;
  std::int32_t n_locals;
  std::int32_t n_captures;
  std::int32_t reserved_1;

  [[nodiscard]] static op_begin_closure_t* reinterpret(op::word_t* w);
};

struct op_closure_header_t {
  op::handler_fn_t handler;
  std::int32_t target;
  std::int32_t reserved_1;
  std::int32_t reserved_2;
  std::int32_t n_captured;

  [[nodiscard]] static op_closure_header_t* reinterpret(op::word_t* w);
};

struct closure_capture_t {
  std::int32_t kind;
  std::int32_t index;

  [[nodiscard]] static const closure_capture_t* reinterpret(const op::word_t* w);
};

struct op_closure_view_t {
  op_closure_header_t* header;
  std::span<closure_capture_t> captures;

  [[nodiscard]] static op_closure_view_t reinterpret(op::word_t* w);
};

struct op_callc_t {
  op::handler_fn_t handler;
  std::int32_t n_args;
  std::int32_t reserved_1;

  [[nodiscard]] static op_callc_t* reinterpret(op::word_t* w);
};

struct op_call_t {
  op::handler_fn_t handler;
  std::int32_t target;
  std::int32_t reserved_1;
  std::int32_t reserved_2;
  std::int32_t n_args;

  [[nodiscard]] static op_call_t* reinterpret(op::word_t* w);
};

struct op_tag_t {
  op::handler_fn_t handler;
  std::int32_t tag_offset;
  std::int32_t reserved_1;
  std::int32_t n_fields;
  std::int32_t reserved_2;

  [[nodiscard]] static op_tag_t* reinterpret(op::word_t* w);
};

struct op_array_t {
  op::handler_fn_t handler;
  std::int32_t n_elements;
  std::int32_t reserved_1;

  [[nodiscard]] static op_array_t* reinterpret(op::word_t* w);
};

struct op_fail_t {
  op::handler_fn_t handler;
  std::int32_t line;
  std::int32_t column;

  [[nodiscard]] static op_fail_t* reinterpret(op::word_t* w);
};

struct op_fail_keep_t {
  op::handler_fn_t handler;
  std::int32_t line;
  std::int32_t column;

  [[nodiscard]] static op_fail_keep_t* reinterpret(op::word_t* w);
};

struct op_line_t {
  op::handler_fn_t handler;
  std::int32_t line;
  std::int32_t reserved_1;

  [[nodiscard]] static op_line_t* reinterpret(op::word_t* w);
};

struct op_patt_str_cmp_t {
  op::handler_fn_t handler;

  [[nodiscard]] static op_patt_str_cmp_t* reinterpret(op::word_t* w);
};

struct op_patt_string_t {
  op::handler_fn_t handler;

  [[nodiscard]] static op_patt_string_t* reinterpret(op::word_t* w);
};

struct op_patt_array_t {
  op::handler_fn_t handler;

  [[nodiscard]] static op_patt_array_t* reinterpret(op::word_t* w);
};

struct op_patt_sexp_t {
  op::handler_fn_t handler;

  [[nodiscard]] static op_patt_sexp_t* reinterpret(op::word_t* w);
};

struct op_patt_boxed_t {
  op::handler_fn_t handler;

  [[nodiscard]] static op_patt_boxed_t* reinterpret(op::word_t* w);
};

struct op_patt_unboxed_t {
  op::handler_fn_t handler;

  [[nodiscard]] static op_patt_unboxed_t* reinterpret(op::word_t* w);
};

struct op_patt_closure_t {
  op::handler_fn_t handler;

  [[nodiscard]] static op_patt_closure_t* reinterpret(op::word_t* w);
};

struct op_barray_t {
  op::handler_fn_t handler;
  std::int32_t n_elements;
  std::int32_t reserved_1;

  [[nodiscard]] static op_barray_t* reinterpret(op::word_t* w);
};

struct op_eof_t {
  op::handler_fn_t handler;

  [[nodiscard]] static op_eof_t* reinterpret(op::word_t* w);
};

static_assert(ctype<op_add_t>);
static_assert(ctype<op_sub_t>);
static_assert(ctype<op_mul_t>);
static_assert(ctype<op_div_t>);
static_assert(ctype<op_mod_t>);
static_assert(ctype<op_lt_t>);
static_assert(ctype<op_le_t>);
static_assert(ctype<op_gt_t>);
static_assert(ctype<op_ge_t>);
static_assert(ctype<op_eq_t>);
static_assert(ctype<op_ne_t>);
static_assert(ctype<op_and_t>);
static_assert(ctype<op_or_t>);
static_assert(ctype<op_const_t>);
static_assert(ctype<op_string_t>);
static_assert(ctype<op_sexp_t>);
static_assert(ctype<op_sta_t>);
static_assert(ctype<op_jmp_t>);
static_assert(ctype<op_end_t>);
static_assert(ctype<op_drop_t>);
static_assert(ctype<op_dup_t>);
static_assert(ctype<op_swap_t>);
static_assert(ctype<op_elem_t>);
static_assert(ctype<op_ld_glo_t>);
static_assert(ctype<op_ld_loc_t>);
static_assert(ctype<op_ld_arg_t>);
static_assert(ctype<op_ld_clo_t>);
static_assert(ctype<op_st_glo_t>);
static_assert(ctype<op_st_loc_t>);
static_assert(ctype<op_st_arg_t>);
static_assert(ctype<op_st_clo_t>);
static_assert(ctype<op_cjmp_z_t>);
static_assert(ctype<op_cjmp_nz_t>);
static_assert(ctype<op_begin_t>);
static_assert(ctype<op_begin_closure_t>);
static_assert(ctype<op_closure_header_t>);
static_assert(ctype<closure_capture_t>);
static_assert(sizeof(closure_capture_t) == sizeof(op::word_t));
static_assert(ctype<op_callc_t>);
static_assert(ctype<op_call_t>);
static_assert(ctype<op_tag_t>);
static_assert(ctype<op_array_t>);
static_assert(ctype<op_fail_t>);
static_assert(ctype<op_fail_keep_t>);
static_assert(ctype<op_line_t>);
static_assert(ctype<op_patt_str_cmp_t>);
static_assert(ctype<op_patt_string_t>);
static_assert(ctype<op_patt_array_t>);
static_assert(ctype<op_patt_sexp_t>);
static_assert(ctype<op_patt_boxed_t>);
static_assert(ctype<op_patt_unboxed_t>);
static_assert(ctype<op_patt_closure_t>);
static_assert(ctype<op_barray_t>);
static_assert(ctype<op_eof_t>);

using op::word_count;

[[nodiscard]] std::ptrdiff_t word_count(const op_closure_view_t& closure);

} // namespace lamavm::interpreter::cold
