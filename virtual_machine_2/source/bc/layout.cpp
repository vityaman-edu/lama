#include "layout.hpp"

#include "core/format.hpp"

#include <format>

LAMAVM_FORMAT_DEFINE(lamavm::bc::public_symbol_t, x, ctx) {
  return LAMAVM_FORMAT_DERIVE(ctx, "public_symbol {} {} {}", x.name_offset, x.code_offset, x.flag);
}

LAMAVM_FORMAT_DEFINE(lamavm::bc::import_t, x, ctx) {
  return LAMAVM_FORMAT_DERIVE(ctx, "import {}", x.name_offset);
}

LAMAVM_FORMAT_DEFINE(lamavm::bc::opcode_t, x, ctx) {
  using lamavm::bc::opcode_t;

  switch (x) {
  case opcode_t::op_add:
    return LAMAVM_FORMAT_DERIVE(ctx, "ADD");
  case opcode_t::op_sub:
    return LAMAVM_FORMAT_DERIVE(ctx, "SUB");
  case opcode_t::op_mul:
    return LAMAVM_FORMAT_DERIVE(ctx, "MUL");
  case opcode_t::op_div:
    return LAMAVM_FORMAT_DERIVE(ctx, "DIV");
  case opcode_t::op_mod:
    return LAMAVM_FORMAT_DERIVE(ctx, "MOD");
  case opcode_t::op_lt:
    return LAMAVM_FORMAT_DERIVE(ctx, "LT");
  case opcode_t::op_le:
    return LAMAVM_FORMAT_DERIVE(ctx, "LE");
  case opcode_t::op_gt:
    return LAMAVM_FORMAT_DERIVE(ctx, "GT");
  case opcode_t::op_ge:
    return LAMAVM_FORMAT_DERIVE(ctx, "GE");
  case opcode_t::op_eq:
    return LAMAVM_FORMAT_DERIVE(ctx, "EQ");
  case opcode_t::op_ne:
    return LAMAVM_FORMAT_DERIVE(ctx, "NE");
  case opcode_t::op_and:
    return LAMAVM_FORMAT_DERIVE(ctx, "AND");
  case opcode_t::op_or:
    return LAMAVM_FORMAT_DERIVE(ctx, "OR");
  case opcode_t::op_const:
    return LAMAVM_FORMAT_DERIVE(ctx, "CONST");
  case opcode_t::op_string:
    return LAMAVM_FORMAT_DERIVE(ctx, "STRING");
  case opcode_t::op_sexp:
    return LAMAVM_FORMAT_DERIVE(ctx, "SEXP");
  case opcode_t::op_sta:
    return LAMAVM_FORMAT_DERIVE(ctx, "STA");
  case opcode_t::op_jmp:
    return LAMAVM_FORMAT_DERIVE(ctx, "JMP");
  case opcode_t::op_end:
    return LAMAVM_FORMAT_DERIVE(ctx, "END");
  case opcode_t::op_drop:
    return LAMAVM_FORMAT_DERIVE(ctx, "DROP");
  case opcode_t::op_dup:
    return LAMAVM_FORMAT_DERIVE(ctx, "DUP");
  case opcode_t::op_swap:
    return LAMAVM_FORMAT_DERIVE(ctx, "SWAP");
  case opcode_t::op_elem:
    return LAMAVM_FORMAT_DERIVE(ctx, "ELEM");
  case opcode_t::op_ld_glo:
    return LAMAVM_FORMAT_DERIVE(ctx, "LD_GLO");
  case opcode_t::op_ld_loc:
    return LAMAVM_FORMAT_DERIVE(ctx, "LD_LOC");
  case opcode_t::op_ld_arg:
    return LAMAVM_FORMAT_DERIVE(ctx, "LD_ARG");
  case opcode_t::op_ld_clo:
    return LAMAVM_FORMAT_DERIVE(ctx, "LD_CLO");
  case opcode_t::op_st_glo:
    return LAMAVM_FORMAT_DERIVE(ctx, "ST_GLO");
  case opcode_t::op_st_loc:
    return LAMAVM_FORMAT_DERIVE(ctx, "ST_LOC");
  case opcode_t::op_st_arg:
    return LAMAVM_FORMAT_DERIVE(ctx, "ST_ARG");
  case opcode_t::op_st_clo:
    return LAMAVM_FORMAT_DERIVE(ctx, "ST_CLO");
  case opcode_t::op_cjmp_z:
    return LAMAVM_FORMAT_DERIVE(ctx, "CJMP_Z");
  case opcode_t::op_cjmp_nz:
    return LAMAVM_FORMAT_DERIVE(ctx, "CJMP_NZ");
  case opcode_t::op_begin:
    return LAMAVM_FORMAT_DERIVE(ctx, "BEGIN");
  case opcode_t::op_begin_closure:
    return LAMAVM_FORMAT_DERIVE(ctx, "BEGIN_CLOSURE");
  case opcode_t::op_closure:
    return LAMAVM_FORMAT_DERIVE(ctx, "CLOSURE");
  case opcode_t::op_callc:
    return LAMAVM_FORMAT_DERIVE(ctx, "CALLC");
  case opcode_t::op_call:
    return LAMAVM_FORMAT_DERIVE(ctx, "CALL");
  case opcode_t::op_tag:
    return LAMAVM_FORMAT_DERIVE(ctx, "TAG");
  case opcode_t::op_array:
    return LAMAVM_FORMAT_DERIVE(ctx, "ARRAY");
  case opcode_t::op_fail:
    return LAMAVM_FORMAT_DERIVE(ctx, "FAIL");
  case opcode_t::op_fail_keep:
    return LAMAVM_FORMAT_DERIVE(ctx, "FAIL_KEEP");
  case opcode_t::op_line:
    return LAMAVM_FORMAT_DERIVE(ctx, "LINE");
  case opcode_t::op_patt_str_cmp:
    return LAMAVM_FORMAT_DERIVE(ctx, "PATT_STR_CMP");
  case opcode_t::op_patt_string:
    return LAMAVM_FORMAT_DERIVE(ctx, "PATT_STRING");
  case opcode_t::op_patt_array:
    return LAMAVM_FORMAT_DERIVE(ctx, "PATT_ARRAY");
  case opcode_t::op_patt_sexp:
    return LAMAVM_FORMAT_DERIVE(ctx, "PATT_SEXP");
  case opcode_t::op_patt_boxed:
    return LAMAVM_FORMAT_DERIVE(ctx, "PATT_BOXED");
  case opcode_t::op_patt_unboxed:
    return LAMAVM_FORMAT_DERIVE(ctx, "PATT_UNBOXED");
  case opcode_t::op_patt_closure:
    return LAMAVM_FORMAT_DERIVE(ctx, "PATT_CLOSURE");
  case opcode_t::op_barray:
    return LAMAVM_FORMAT_DERIVE(ctx, "BARRAY");
  case opcode_t::op_eof:
    return LAMAVM_FORMAT_DERIVE(ctx, "EOF");
  }

  return LAMAVM_FORMAT_DERIVE(ctx, "{:x}", static_cast<unsigned int>(x));
}

LAMAVM_FORMAT_DEFINE(lamavm::bc::op_begin_t, x, ctx) {
  return LAMAVM_FORMAT_DERIVE(ctx, "{} {} {}", x.opcode, x.n_args, x.n_locals);
}

LAMAVM_FORMAT_DEFINE(lamavm::bc::op_const_t, x, ctx) {
  return LAMAVM_FORMAT_DERIVE(ctx, "{} {}", x.opcode, x.value);
}

LAMAVM_FORMAT_DEFINE(lamavm::bc::op_end_t, x, ctx) {
  return LAMAVM_FORMAT_DERIVE(ctx, "{}", x.opcode);
}

LAMAVM_FORMAT_DEFINE(lamavm::bc::op_eof_t, x, ctx) {
  return LAMAVM_FORMAT_DERIVE(ctx, "{}", x.opcode);
}

LAMAVM_FORMAT_DEFINE(lamavm::bc::op_binop_t, x, ctx) {
  return LAMAVM_FORMAT_DERIVE(ctx, "{}", x.opcode);
}

LAMAVM_FORMAT_DEFINE(lamavm::bc::op_string_t, x, ctx) {
  return LAMAVM_FORMAT_DERIVE(ctx, "{} {}", x.opcode, x.string_offset);
}

LAMAVM_FORMAT_DEFINE(lamavm::bc::op_jmp_t, x, ctx) {
  return LAMAVM_FORMAT_DERIVE(ctx, "{} {}", x.opcode, x.target);
}

LAMAVM_FORMAT_DEFINE(lamavm::bc::op_cjmp_t, x, ctx) {
  return LAMAVM_FORMAT_DERIVE(ctx, "{} {}", x.opcode, x.target);
}

LAMAVM_FORMAT_DEFINE(lamavm::bc::op_drop_t, x, ctx) {
  return LAMAVM_FORMAT_DERIVE(ctx, "{}", x.opcode);
}

LAMAVM_FORMAT_DEFINE(lamavm::bc::op_dup_t, x, ctx) {
  return LAMAVM_FORMAT_DERIVE(ctx, "{}", x.opcode);
}

LAMAVM_FORMAT_DEFINE(lamavm::bc::op_swap_t, x, ctx) {
  return LAMAVM_FORMAT_DERIVE(ctx, "{}", x.opcode);
}

LAMAVM_FORMAT_DEFINE(lamavm::bc::op_call_t, x, ctx) {
  return LAMAVM_FORMAT_DERIVE(ctx, "{} {} {}", x.opcode, x.target, x.n_args);
}

LAMAVM_FORMAT_DEFINE(lamavm::bc::op_ld_loc_t, x, ctx) {
  return LAMAVM_FORMAT_DERIVE(ctx, "{} {}", x.opcode, x.index);
}

LAMAVM_FORMAT_DEFINE(lamavm::bc::op_ld_arg_t, x, ctx) {
  return LAMAVM_FORMAT_DERIVE(ctx, "{} {}", x.opcode, x.index);
}

LAMAVM_FORMAT_DEFINE(lamavm::bc::op_st_loc_t, x, ctx) {
  return LAMAVM_FORMAT_DERIVE(ctx, "{} {}", x.opcode, x.index);
}

LAMAVM_FORMAT_DEFINE(lamavm::bc::op_st_arg_t, x, ctx) {
  return LAMAVM_FORMAT_DERIVE(ctx, "{} {}", x.opcode, x.index);
}

LAMAVM_FORMAT_DEFINE(lamavm::bc::op_ld_glo_t, x, ctx) {
  return LAMAVM_FORMAT_DERIVE(ctx, "{} {}", x.opcode, x.index);
}

LAMAVM_FORMAT_DEFINE(lamavm::bc::op_st_glo_t, x, ctx) {
  return LAMAVM_FORMAT_DERIVE(ctx, "{} {}", x.opcode, x.index);
}
