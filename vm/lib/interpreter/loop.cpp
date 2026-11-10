#include "loop.hpp"

#include "bc/layout.hpp"
#include "core/exception.hpp"

#include <span>

namespace lamavm::interpreter {

void loop::operator()(const bc::add &op) {
  (void)op;
  throw lamavm::exception("unsupported instruction");
}

void loop::operator()(const bc::sub &op) {
  (void)op;
  throw lamavm::exception("unsupported instruction");
}

void loop::operator()(const bc::mul &op) {
  (void)op;
  throw lamavm::exception("unsupported instruction");
}

void loop::operator()(const bc::div &op) {
  (void)op;
  throw lamavm::exception("unsupported instruction");
}

void loop::operator()(const bc::mod &op) {
  (void)op;
  throw lamavm::exception("unsupported instruction");
}

void loop::operator()(const bc::lt &op) {
  (void)op;
  throw lamavm::exception("unsupported instruction");
}

void loop::operator()(const bc::le &op) {
  (void)op;
  throw lamavm::exception("unsupported instruction");
}

void loop::operator()(const bc::gt &op) {
  (void)op;
  throw lamavm::exception("unsupported instruction");
}

void loop::operator()(const bc::ge &op) {
  (void)op;
  throw lamavm::exception("unsupported instruction");
}

void loop::operator()(const bc::eq &op) {
  (void)op;
  throw lamavm::exception("unsupported instruction");
}

void loop::operator()(const bc::ne &op) {
  (void)op;
  throw lamavm::exception("unsupported instruction");
}

void loop::operator()(const bc::logical_and &op) {
  (void)op;
  throw lamavm::exception("unsupported instruction");
}

void loop::operator()(const bc::logical_or &op) {
  (void)op;
  throw lamavm::exception("unsupported instruction");
}

void loop::operator()(const bc::constant &op) {
  (void)op;
  throw lamavm::exception("unsupported instruction");
}

void loop::operator()(const bc::string &op) {
  (void)op;
  throw lamavm::exception("unsupported instruction");
}

void loop::operator()(const bc::sexp &op) {
  (void)op;
  throw lamavm::exception("unsupported instruction");
}

void loop::operator()(const bc::sti &op) {
  (void)op;
  throw lamavm::exception("unsupported instruction");
}

void loop::operator()(const bc::sta &op) {
  (void)op;
  throw lamavm::exception("unsupported instruction");
}

void loop::operator()(const bc::jmp &op) {
  (void)op;
  throw lamavm::exception("unsupported instruction");
}

void loop::operator()(const bc::end &op) {
  (void)op;
  throw lamavm::exception("unsupported instruction");
}

void loop::operator()(const bc::ret &op) {
  (void)op;
  throw lamavm::exception("unsupported instruction");
}

void loop::operator()(const bc::drop &op) {
  (void)op;
  throw lamavm::exception("unsupported instruction");
}

void loop::operator()(const bc::dup &op) {
  (void)op;
  throw lamavm::exception("unsupported instruction");
}

void loop::operator()(const bc::swap &op) {
  (void)op;
  throw lamavm::exception("unsupported instruction");
}

void loop::operator()(const bc::elem &op) {
  (void)op;
  throw lamavm::exception("unsupported instruction");
}

void loop::operator()(const bc::ld_glo &op) {
  (void)op;
  throw lamavm::exception("unsupported instruction");
}

void loop::operator()(const bc::ld_loc &op) {
  (void)op;
  throw lamavm::exception("unsupported instruction");
}

void loop::operator()(const bc::ld_arg &op) {
  (void)op;
  throw lamavm::exception("unsupported instruction");
}

void loop::operator()(const bc::ld_clo &op) {
  (void)op;
  throw lamavm::exception("unsupported instruction");
}

void loop::operator()(const bc::lda_glo &op) {
  (void)op;
  throw lamavm::exception("unsupported instruction");
}

void loop::operator()(const bc::lda_loc &op) {
  (void)op;
  throw lamavm::exception("unsupported instruction");
}

void loop::operator()(const bc::lda_arg &op) {
  (void)op;
  throw lamavm::exception("unsupported instruction");
}

void loop::operator()(const bc::lda_clo &op) {
  (void)op;
  throw lamavm::exception("unsupported instruction");
}

void loop::operator()(const bc::st_glo &op) {
  (void)op;
  throw lamavm::exception("unsupported instruction");
}

void loop::operator()(const bc::st_loc &op) {
  (void)op;
  throw lamavm::exception("unsupported instruction");
}

void loop::operator()(const bc::st_arg &op) {
  (void)op;
  throw lamavm::exception("unsupported instruction");
}

void loop::operator()(const bc::st_clo &op) {
  (void)op;
  throw lamavm::exception("unsupported instruction");
}

void loop::operator()(const bc::cjmp_z &op) {
  (void)op;
  throw lamavm::exception("unsupported instruction");
}

void loop::operator()(const bc::cjmp_nz &op) {
  (void)op;
  throw lamavm::exception("unsupported instruction");
}

void loop::operator()(const bc::begin &op) {
  (void)op;
  throw lamavm::exception("unsupported instruction");
}

void loop::operator()(const bc::begin_closure &op) {
  (void)op;
  throw lamavm::exception("unsupported instruction");
}

void loop::operator()(const bc::closure &op, std::span<const bc::closure::capture> captures) {
  (void)op;
  (void)captures;
  throw lamavm::exception("unsupported instruction");
}

void loop::operator()(const bc::callc &op) {
  (void)op;
  throw lamavm::exception("unsupported instruction");
}

void loop::operator()(const bc::call &op) {
  (void)op;
  throw lamavm::exception("unsupported instruction");
}

void loop::operator()(const bc::tag &op) {
  (void)op;
  throw lamavm::exception("unsupported instruction");
}

void loop::operator()(const bc::array &op) {
  (void)op;
  throw lamavm::exception("unsupported instruction");
}

void loop::operator()(const bc::fail &op) {
  (void)op;
  throw lamavm::exception("unsupported instruction");
}

void loop::operator()(const bc::fail_keep &op) {
  (void)op;
  throw lamavm::exception("unsupported instruction");
}

void loop::operator()(const bc::line &op) {
  (void)op;
  throw lamavm::exception("unsupported instruction");
}

void loop::operator()(const bc::patt_str_cmp &op) {
  (void)op;
  throw lamavm::exception("unsupported instruction");
}

void loop::operator()(const bc::patt_string &op) {
  (void)op;
  throw lamavm::exception("unsupported instruction");
}

void loop::operator()(const bc::patt_array &op) {
  (void)op;
  throw lamavm::exception("unsupported instruction");
}

void loop::operator()(const bc::patt_sexp &op) {
  (void)op;
  throw lamavm::exception("unsupported instruction");
}

void loop::operator()(const bc::patt_boxed &op) {
  (void)op;
  throw lamavm::exception("unsupported instruction");
}

void loop::operator()(const bc::patt_unboxed &op) {
  (void)op;
  throw lamavm::exception("unsupported instruction");
}

void loop::operator()(const bc::patt_closure &op) {
  (void)op;
  throw lamavm::exception("unsupported instruction");
}

void loop::operator()(const bc::barray &op) {
  (void)op;
  throw lamavm::exception("unsupported instruction");
}

void loop::operator()(const bc::eof &op) {
  (void)op;
  throw lamavm::exception("unsupported instruction");
}

} // namespace lamavm::interpreter
