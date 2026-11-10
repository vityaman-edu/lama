#include "loop.hpp"

#include "bc/layout.hpp"
#include "core/exception.hpp"

#include <span>

namespace lamavm::bc::interpreter {

void loop::operator()(const add &op) {
  (void)op;
  throw lamavm::exception("unsupported instruction");
}

void loop::operator()(const sub &op) {
  (void)op;
  throw lamavm::exception("unsupported instruction");
}

void loop::operator()(const mul &op) {
  (void)op;
  throw lamavm::exception("unsupported instruction");
}

void loop::operator()(const div &op) {
  (void)op;
  throw lamavm::exception("unsupported instruction");
}

void loop::operator()(const mod &op) {
  (void)op;
  throw lamavm::exception("unsupported instruction");
}

void loop::operator()(const lt &op) {
  (void)op;
  throw lamavm::exception("unsupported instruction");
}

void loop::operator()(const le &op) {
  (void)op;
  throw lamavm::exception("unsupported instruction");
}

void loop::operator()(const gt &op) {
  (void)op;
  throw lamavm::exception("unsupported instruction");
}

void loop::operator()(const ge &op) {
  (void)op;
  throw lamavm::exception("unsupported instruction");
}

void loop::operator()(const eq &op) {
  (void)op;
  throw lamavm::exception("unsupported instruction");
}

void loop::operator()(const ne &op) {
  (void)op;
  throw lamavm::exception("unsupported instruction");
}

void loop::operator()(const logical_and &op) {
  (void)op;
  throw lamavm::exception("unsupported instruction");
}

void loop::operator()(const logical_or &op) {
  (void)op;
  throw lamavm::exception("unsupported instruction");
}

void loop::operator()(const constant &op) {
  (void)op;
  throw lamavm::exception("unsupported instruction");
}

void loop::operator()(const string &op) {
  (void)op;
  throw lamavm::exception("unsupported instruction");
}

void loop::operator()(const sexp &op) {
  (void)op;
  throw lamavm::exception("unsupported instruction");
}

void loop::operator()(const sti &op) {
  (void)op;
  throw lamavm::exception("unsupported instruction");
}

void loop::operator()(const sta &op) {
  (void)op;
  throw lamavm::exception("unsupported instruction");
}

void loop::operator()(const jmp &op) {
  (void)op;
  throw lamavm::exception("unsupported instruction");
}

void loop::operator()(const end &op) {
  (void)op;
  throw lamavm::exception("unsupported instruction");
}

void loop::operator()(const ret &op) {
  (void)op;
  throw lamavm::exception("unsupported instruction");
}

void loop::operator()(const drop &op) {
  (void)op;
  throw lamavm::exception("unsupported instruction");
}

void loop::operator()(const dup &op) {
  (void)op;
  throw lamavm::exception("unsupported instruction");
}

void loop::operator()(const swap &op) {
  (void)op;
  throw lamavm::exception("unsupported instruction");
}

void loop::operator()(const elem &op) {
  (void)op;
  throw lamavm::exception("unsupported instruction");
}

void loop::operator()(const ld_glo &op) {
  (void)op;
  throw lamavm::exception("unsupported instruction");
}

void loop::operator()(const ld_loc &op) {
  (void)op;
  throw lamavm::exception("unsupported instruction");
}

void loop::operator()(const ld_arg &op) {
  (void)op;
  throw lamavm::exception("unsupported instruction");
}

void loop::operator()(const ld_clo &op) {
  (void)op;
  throw lamavm::exception("unsupported instruction");
}

void loop::operator()(const lda_glo &op) {
  (void)op;
  throw lamavm::exception("unsupported instruction");
}

void loop::operator()(const lda_loc &op) {
  (void)op;
  throw lamavm::exception("unsupported instruction");
}

void loop::operator()(const lda_arg &op) {
  (void)op;
  throw lamavm::exception("unsupported instruction");
}

void loop::operator()(const lda_clo &op) {
  (void)op;
  throw lamavm::exception("unsupported instruction");
}

void loop::operator()(const st_glo &op) {
  (void)op;
  throw lamavm::exception("unsupported instruction");
}

void loop::operator()(const st_loc &op) {
  (void)op;
  throw lamavm::exception("unsupported instruction");
}

void loop::operator()(const st_arg &op) {
  (void)op;
  throw lamavm::exception("unsupported instruction");
}

void loop::operator()(const st_clo &op) {
  (void)op;
  throw lamavm::exception("unsupported instruction");
}

void loop::operator()(const cjmp_z &op) {
  (void)op;
  throw lamavm::exception("unsupported instruction");
}

void loop::operator()(const cjmp_nz &op) {
  (void)op;
  throw lamavm::exception("unsupported instruction");
}

void loop::operator()(const begin &op) {
  (void)op;
  throw lamavm::exception("unsupported instruction");
}

void loop::operator()(const begin_closure &op) {
  (void)op;
  throw lamavm::exception("unsupported instruction");
}

void loop::operator()(const closure &op, std::span<const closure::capture> captures) {
  (void)op;
  (void)captures;
  throw lamavm::exception("unsupported instruction");
}

void loop::operator()(const callc &op) {
  (void)op;
  throw lamavm::exception("unsupported instruction");
}

void loop::operator()(const call &op) {
  (void)op;
  throw lamavm::exception("unsupported instruction");
}

void loop::operator()(const tag &op) {
  (void)op;
  throw lamavm::exception("unsupported instruction");
}

void loop::operator()(const array &op) {
  (void)op;
  throw lamavm::exception("unsupported instruction");
}

void loop::operator()(const fail &op) {
  (void)op;
  throw lamavm::exception("unsupported instruction");
}

void loop::operator()(const fail_keep &op) {
  (void)op;
  throw lamavm::exception("unsupported instruction");
}

void loop::operator()(const line &op) {
  (void)op;
  throw lamavm::exception("unsupported instruction");
}

void loop::operator()(const patt_str_cmp &op) {
  (void)op;
  throw lamavm::exception("unsupported instruction");
}

void loop::operator()(const patt_string &op) {
  (void)op;
  throw lamavm::exception("unsupported instruction");
}

void loop::operator()(const patt_array &op) {
  (void)op;
  throw lamavm::exception("unsupported instruction");
}

void loop::operator()(const patt_sexp &op) {
  (void)op;
  throw lamavm::exception("unsupported instruction");
}

void loop::operator()(const patt_boxed &op) {
  (void)op;
  throw lamavm::exception("unsupported instruction");
}

void loop::operator()(const patt_unboxed &op) {
  (void)op;
  throw lamavm::exception("unsupported instruction");
}

void loop::operator()(const patt_closure &op) {
  (void)op;
  throw lamavm::exception("unsupported instruction");
}

void loop::operator()(const barray &op) {
  (void)op;
  throw lamavm::exception("unsupported instruction");
}

void loop::operator()(const eof &op) {
  (void)op;
  throw lamavm::exception("unsupported instruction");
}

} // namespace lamavm::bc::interpreter
