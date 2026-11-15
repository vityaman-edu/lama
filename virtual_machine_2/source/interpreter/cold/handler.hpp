#pragma once

namespace lamavm::interpreter::cold {

void op_add();
void op_sub();
void op_mul();
void op_div();
void op_mod();
void op_lt();
void op_le();
void op_gt();
void op_ge();
void op_eq();
void op_ne();
void op_and();
void op_or();
void op_const();
void op_string();
void op_sexp();
void op_drop();
void op_dup();
void op_swap();
void op_ld_glo();
void op_ld_loc();
void op_ld_arg();
void op_st_glo();
void op_st_loc();
void op_st_arg();
void op_jmp();
void op_cjmp_z();
void op_cjmp_nz();
void op_call();
void op_begin();
void op_end();
void op_line();
void op_eof();

} // namespace lamavm::interpreter::cold
