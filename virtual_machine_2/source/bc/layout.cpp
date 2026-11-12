#include "layout.hpp"

#include <ostream>

namespace lamavm::bc {

std::ostream& operator<<(std::ostream& os, opcode_t opcode) {
  switch (opcode) {
  case opcode_t::op_add:
    return os << "op_add";
  case opcode_t::op_sub:
    return os << "op_sub";
  case opcode_t::op_mul:
    return os << "op_mul";
  case opcode_t::op_div:
    return os << "op_div";
  case opcode_t::op_mod:
    return os << "op_mod";
  case opcode_t::op_lt:
    return os << "op_lt";
  case opcode_t::op_le:
    return os << "op_le";
  case opcode_t::op_gt:
    return os << "op_gt";
  case opcode_t::op_ge:
    return os << "op_ge";
  case opcode_t::op_eq:
    return os << "op_eq";
  case opcode_t::op_ne:
    return os << "op_ne";
  case opcode_t::op_and:
    return os << "op_and";
  case opcode_t::op_or:
    return os << "op_or";
  case opcode_t::op_const:
    return os << "op_const";
  case opcode_t::op_string:
    return os << "op_string";
  case opcode_t::op_sexp:
    return os << "op_sexp";
  case opcode_t::op_sta:
    return os << "op_sta";
  case opcode_t::op_jmp:
    return os << "op_jmp";
  case opcode_t::op_end:
    return os << "op_end";
  case opcode_t::op_drop:
    return os << "op_drop";
  case opcode_t::op_dup:
    return os << "op_dup";
  case opcode_t::op_swap:
    return os << "op_swap";
  case opcode_t::op_elem:
    return os << "op_elem";
  case opcode_t::op_ld_glo:
    return os << "op_ld_glo";
  case opcode_t::op_ld_loc:
    return os << "op_ld_loc";
  case opcode_t::op_ld_arg:
    return os << "op_ld_arg";
  case opcode_t::op_ld_clo:
    return os << "op_ld_clo";
  case opcode_t::op_st_glo:
    return os << "op_st_glo";
  case opcode_t::op_st_loc:
    return os << "op_st_loc";
  case opcode_t::op_st_arg:
    return os << "op_st_arg";
  case opcode_t::op_st_clo:
    return os << "op_st_clo";
  case opcode_t::op_cjmp_z:
    return os << "op_cjmp_z";
  case opcode_t::op_cjmp_nz:
    return os << "op_cjmp_nz";
  case opcode_t::op_begin:
    return os << "op_begin";
  case opcode_t::op_begin_closure:
    return os << "op_begin_closure";
  case opcode_t::op_closure:
    return os << "op_closure";
  case opcode_t::op_callc:
    return os << "op_callc";
  case opcode_t::op_call:
    return os << "op_call";
  case opcode_t::op_tag:
    return os << "op_tag";
  case opcode_t::op_array:
    return os << "op_array";
  case opcode_t::op_fail:
    return os << "op_fail";
  case opcode_t::op_fail_keep:
    return os << "op_fail_keep";
  case opcode_t::op_line:
    return os << "op_line";
  case opcode_t::op_patt_str_cmp:
    return os << "op_patt_str_cmp";
  case opcode_t::op_patt_string:
    return os << "op_patt_string";
  case opcode_t::op_patt_array:
    return os << "op_patt_array";
  case opcode_t::op_patt_sexp:
    return os << "op_patt_sexp";
  case opcode_t::op_patt_boxed:
    return os << "op_patt_boxed";
  case opcode_t::op_patt_unboxed:
    return os << "op_patt_unboxed";
  case opcode_t::op_patt_closure:
    return os << "op_patt_closure";
  case opcode_t::op_barray:
    return os << "op_barray";
  case opcode_t::op_eof:
    return os << "op_eof";
  }

  return os << static_cast<unsigned int>(opcode);
}

} // namespace lamavm::bc
