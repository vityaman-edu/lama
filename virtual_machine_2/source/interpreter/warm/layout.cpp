#include "layout.hpp"

#include "op/word.hpp"

#include <cstddef>
#include <iterator>

namespace lamavm::interpreter::warm {

op_add_t* op_add_t::reinterpret(op::word_t* w) {
  return reinterpret_cast<op_add_t*>(w);
}

op_sub_t* op_sub_t::reinterpret(op::word_t* w) {
  return reinterpret_cast<op_sub_t*>(w);
}

op_mul_t* op_mul_t::reinterpret(op::word_t* w) {
  return reinterpret_cast<op_mul_t*>(w);
}

op_div_t* op_div_t::reinterpret(op::word_t* w) {
  return reinterpret_cast<op_div_t*>(w);
}

op_mod_t* op_mod_t::reinterpret(op::word_t* w) {
  return reinterpret_cast<op_mod_t*>(w);
}

op_lt_t* op_lt_t::reinterpret(op::word_t* w) {
  return reinterpret_cast<op_lt_t*>(w);
}

op_le_t* op_le_t::reinterpret(op::word_t* w) {
  return reinterpret_cast<op_le_t*>(w);
}

op_gt_t* op_gt_t::reinterpret(op::word_t* w) {
  return reinterpret_cast<op_gt_t*>(w);
}

op_ge_t* op_ge_t::reinterpret(op::word_t* w) {
  return reinterpret_cast<op_ge_t*>(w);
}

op_eq_t* op_eq_t::reinterpret(op::word_t* w) {
  return reinterpret_cast<op_eq_t*>(w);
}

op_ne_t* op_ne_t::reinterpret(op::word_t* w) {
  return reinterpret_cast<op_ne_t*>(w);
}

op_and_t* op_and_t::reinterpret(op::word_t* w) {
  return reinterpret_cast<op_and_t*>(w);
}

op_or_t* op_or_t::reinterpret(op::word_t* w) {
  return reinterpret_cast<op_or_t*>(w);
}

op_const_t* op_const_t::reinterpret(op::word_t* w) {
  return reinterpret_cast<op_const_t*>(w);
}

op_string_t* op_string_t::reinterpret(op::word_t* w) {
  return reinterpret_cast<op_string_t*>(w);
}

op_sexp_t* op_sexp_t::reinterpret(op::word_t* w) {
  return reinterpret_cast<op_sexp_t*>(w);
}

op_sta_t* op_sta_t::reinterpret(op::word_t* w) {
  return reinterpret_cast<op_sta_t*>(w);
}

op_jmp_t* op_jmp_t::reinterpret(op::word_t* w) {
  return reinterpret_cast<op_jmp_t*>(w);
}

op_end_t* op_end_t::reinterpret(op::word_t* w) {
  return reinterpret_cast<op_end_t*>(w);
}

op_drop_t* op_drop_t::reinterpret(op::word_t* w) {
  return reinterpret_cast<op_drop_t*>(w);
}

op_dup_t* op_dup_t::reinterpret(op::word_t* w) {
  return reinterpret_cast<op_dup_t*>(w);
}

op_swap_t* op_swap_t::reinterpret(op::word_t* w) {
  return reinterpret_cast<op_swap_t*>(w);
}

op_elem_t* op_elem_t::reinterpret(op::word_t* w) {
  return reinterpret_cast<op_elem_t*>(w);
}

op_ld_glo_t* op_ld_glo_t::reinterpret(op::word_t* w) {
  return reinterpret_cast<op_ld_glo_t*>(w);
}

op_ld_arg_t* op_ld_arg_t::reinterpret(op::word_t* w) {
  return reinterpret_cast<op_ld_arg_t*>(w);
}

op_ld_loc_t* op_ld_loc_t::reinterpret(op::word_t* w) {
  return reinterpret_cast<op_ld_loc_t*>(w);
}

op_ld_clo_t* op_ld_clo_t::reinterpret(op::word_t* w) {
  return reinterpret_cast<op_ld_clo_t*>(w);
}

op_st_glo_t* op_st_glo_t::reinterpret(op::word_t* w) {
  return reinterpret_cast<op_st_glo_t*>(w);
}

op_st_arg_t* op_st_arg_t::reinterpret(op::word_t* w) {
  return reinterpret_cast<op_st_arg_t*>(w);
}

op_st_loc_t* op_st_loc_t::reinterpret(op::word_t* w) {
  return reinterpret_cast<op_st_loc_t*>(w);
}

op_st_clo_t* op_st_clo_t::reinterpret(op::word_t* w) {
  return reinterpret_cast<op_st_clo_t*>(w);
}

op_cjmp_z_t* op_cjmp_z_t::reinterpret(op::word_t* w) {
  return reinterpret_cast<op_cjmp_z_t*>(w);
}

op_cjmp_nz_t* op_cjmp_nz_t::reinterpret(op::word_t* w) {
  return reinterpret_cast<op_cjmp_nz_t*>(w);
}

op_begin_t* op_begin_t::reinterpret(op::word_t* w) {
  return reinterpret_cast<op_begin_t*>(w);
}

op_begin_no_locals_t* op_begin_no_locals_t::reinterpret(op::word_t* w) {
  return reinterpret_cast<op_begin_no_locals_t*>(w);
}

op_begin_closure_t* op_begin_closure_t::reinterpret(op::word_t* w) {
  return reinterpret_cast<op_begin_closure_t*>(w);
}

op_begin_closure_no_locals_t* op_begin_closure_no_locals_t::reinterpret(op::word_t* w) {
  return reinterpret_cast<op_begin_closure_no_locals_t*>(w);
}

op_closure_header_t* op_closure_header_t::reinterpret(op::word_t* w) {
  return reinterpret_cast<op_closure_header_t*>(w);
}

op_closure_view_t op_closure_view_t::reinterpret(op::word_t* w) {
  auto* const header = op_closure_header_t::reinterpret(w);
  const auto* const captures =
    reinterpret_cast<const op::capture_t*>(w + word_count<op_closure_header_t>());

  return {.header = header, .captures = {captures, static_cast<std::size_t>(header->n_captured)}};
}

op_callc_t* op_callc_t::reinterpret(op::word_t* w) {
  return reinterpret_cast<op_callc_t*>(w);
}

op_call_t* op_call_t::reinterpret(op::word_t* w) {
  return reinterpret_cast<op_call_t*>(w);
}

op_call_local_t* op_call_local_t::reinterpret(op::word_t* w) {
  return reinterpret_cast<op_call_local_t*>(w);
}

op_call_std_t* op_call_std_t::reinterpret(op::word_t* w) {
  return reinterpret_cast<op_call_std_t*>(w);
}

op_tag_t* op_tag_t::reinterpret(op::word_t* w) {
  return reinterpret_cast<op_tag_t*>(w);
}

op_array_t* op_array_t::reinterpret(op::word_t* w) {
  return reinterpret_cast<op_array_t*>(w);
}

op_line_t* op_line_t::reinterpret(op::word_t* w) {
  return reinterpret_cast<op_line_t*>(w);
}

op_patt_str_cmp_t* op_patt_str_cmp_t::reinterpret(op::word_t* w) {
  return reinterpret_cast<op_patt_str_cmp_t*>(w);
}

op_patt_string_t* op_patt_string_t::reinterpret(op::word_t* w) {
  return reinterpret_cast<op_patt_string_t*>(w);
}

op_patt_array_t* op_patt_array_t::reinterpret(op::word_t* w) {
  return reinterpret_cast<op_patt_array_t*>(w);
}

op_patt_sexp_t* op_patt_sexp_t::reinterpret(op::word_t* w) {
  return reinterpret_cast<op_patt_sexp_t*>(w);
}

op_patt_boxed_t* op_patt_boxed_t::reinterpret(op::word_t* w) {
  return reinterpret_cast<op_patt_boxed_t*>(w);
}

op_patt_unboxed_t* op_patt_unboxed_t::reinterpret(op::word_t* w) {
  return reinterpret_cast<op_patt_unboxed_t*>(w);
}

op_patt_closure_t* op_patt_closure_t::reinterpret(op::word_t* w) {
  return reinterpret_cast<op_patt_closure_t*>(w);
}

op_barray_t* op_barray_t::reinterpret(op::word_t* w) {
  return reinterpret_cast<op_barray_t*>(w);
}

std::ptrdiff_t word_count(const op_closure_view_t& closure) {
  return word_count<op_closure_header_t>() + std::ssize(closure.captures);
}

} // namespace lamavm::interpreter::warm
