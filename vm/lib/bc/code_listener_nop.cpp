#include "code_listener_nop.hpp"

#include "layout.hpp"

#include <span>

namespace lamavm::bc {

void code_listener_nop::operator()(const add &op) {
  (void)op;
}

void code_listener_nop::operator()(const sub &op) {
  (void)op;
}

void code_listener_nop::operator()(const mul &op) {
  (void)op;
}

void code_listener_nop::operator()(const div &op) {
  (void)op;
}

void code_listener_nop::operator()(const mod &op) {
  (void)op;
}

void code_listener_nop::operator()(const lt &op) {
  (void)op;
}

void code_listener_nop::operator()(const le &op) {
  (void)op;
}

void code_listener_nop::operator()(const gt &op) {
  (void)op;
}

void code_listener_nop::operator()(const ge &op) {
  (void)op;
}

void code_listener_nop::operator()(const eq &op) {
  (void)op;
}

void code_listener_nop::operator()(const ne &op) {
  (void)op;
}

void code_listener_nop::operator()(const logical_and &op) {
  (void)op;
}

void code_listener_nop::operator()(const logical_or &op) {
  (void)op;
}

void code_listener_nop::operator()(const constant &op) {
  (void)op;
}

void code_listener_nop::operator()(const string &op) {
  (void)op;
}

void code_listener_nop::operator()(const sexp &op) {
  (void)op;
}

void code_listener_nop::operator()(const sti &op) {
  (void)op;
}

void code_listener_nop::operator()(const sta &op) {
  (void)op;
}

void code_listener_nop::operator()(const jmp &op) {
  (void)op;
}

void code_listener_nop::operator()(const end &op) {
  (void)op;
}

void code_listener_nop::operator()(const ret &op) {
  (void)op;
}

void code_listener_nop::operator()(const drop &op) {
  (void)op;
}

void code_listener_nop::operator()(const dup &op) {
  (void)op;
}

void code_listener_nop::operator()(const swap &op) {
  (void)op;
}

void code_listener_nop::operator()(const elem &op) {
  (void)op;
}

void code_listener_nop::operator()(const ld_glo &op) {
  (void)op;
}

void code_listener_nop::operator()(const ld_loc &op) {
  (void)op;
}

void code_listener_nop::operator()(const ld_arg &op) {
  (void)op;
}

void code_listener_nop::operator()(const ld_clo &op) {
  (void)op;
}

void code_listener_nop::operator()(const lda_glo &op) {
  (void)op;
}

void code_listener_nop::operator()(const lda_loc &op) {
  (void)op;
}

void code_listener_nop::operator()(const lda_arg &op) {
  (void)op;
}

void code_listener_nop::operator()(const lda_clo &op) {
  (void)op;
}

void code_listener_nop::operator()(const st_glo &op) {
  (void)op;
}

void code_listener_nop::operator()(const st_loc &op) {
  (void)op;
}

void code_listener_nop::operator()(const st_arg &op) {
  (void)op;
}

void code_listener_nop::operator()(const st_clo &op) {
  (void)op;
}

void code_listener_nop::operator()(const cjmp_z &op) {
  (void)op;
}

void code_listener_nop::operator()(const cjmp_nz &op) {
  (void)op;
}

void code_listener_nop::operator()(const begin &op) {
  (void)op;
}

void code_listener_nop::operator()(const begin_closure &op) {
  (void)op;
}

void code_listener_nop::operator()(const closure &op, std::span<const closure::capture> captures) {
  (void)op;
  (void)captures;
}

void code_listener_nop::operator()(const callc &op) {
  (void)op;
}

void code_listener_nop::operator()(const call &op) {
  (void)op;
}

void code_listener_nop::operator()(const tag &op) {
  (void)op;
}

void code_listener_nop::operator()(const array &op) {
  (void)op;
}

void code_listener_nop::operator()(const fail &op) {
  (void)op;
}

void code_listener_nop::operator()(const fail_keep &op) {
  (void)op;
}

void code_listener_nop::operator()(const line &op) {
  (void)op;
}

void code_listener_nop::operator()(const patt_str_cmp &op) {
  (void)op;
}

void code_listener_nop::operator()(const patt_string &op) {
  (void)op;
}

void code_listener_nop::operator()(const patt_array &op) {
  (void)op;
}

void code_listener_nop::operator()(const patt_sexp &op) {
  (void)op;
}

void code_listener_nop::operator()(const patt_boxed &op) {
  (void)op;
}

void code_listener_nop::operator()(const patt_unboxed &op) {
  (void)op;
}

void code_listener_nop::operator()(const patt_closure &op) {
  (void)op;
}

void code_listener_nop::operator()(const barray &op) {
  (void)op;
}

void code_listener_nop::operator()(const eof &op) {
  (void)op;
}

} // namespace lamavm::bc
