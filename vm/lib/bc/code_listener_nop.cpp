#include "code_listener_nop.hpp"

#include "core/exception.hpp"
#include "layout.hpp"

#include <span>

namespace lamavm::bc {

auto code_listener_nop::operator()(const add &op) -> status {
  (void)op;

  return {};
}

auto code_listener_nop::operator()(const sub &op) -> status {
  (void)op;

  return {};
}

auto code_listener_nop::operator()(const mul &op) -> status {
  (void)op;

  return {};
}

auto code_listener_nop::operator()(const div &op) -> status {
  (void)op;

  return {};
}

auto code_listener_nop::operator()(const mod &op) -> status {
  (void)op;

  return {};
}

auto code_listener_nop::operator()(const lt &op) -> status {
  (void)op;

  return {};
}

auto code_listener_nop::operator()(const le &op) -> status {
  (void)op;

  return {};
}

auto code_listener_nop::operator()(const gt &op) -> status {
  (void)op;

  return {};
}

auto code_listener_nop::operator()(const ge &op) -> status {
  (void)op;

  return {};
}

auto code_listener_nop::operator()(const eq &op) -> status {
  (void)op;

  return {};
}

auto code_listener_nop::operator()(const ne &op) -> status {
  (void)op;

  return {};
}

auto code_listener_nop::operator()(const logical_and &op) -> status {
  (void)op;

  return {};
}

auto code_listener_nop::operator()(const logical_or &op) -> status {
  (void)op;

  return {};
}

auto code_listener_nop::operator()(const constant &op) -> status {
  (void)op;

  return {};
}

auto code_listener_nop::operator()(const string &op) -> status {
  (void)op;

  return {};
}

auto code_listener_nop::operator()(const sexp &op) -> status {
  (void)op;

  return {};
}

auto code_listener_nop::operator()(const sti &op) -> status {
  (void)op;

  return {};
}

auto code_listener_nop::operator()(const sta &op) -> status {
  (void)op;

  return {};
}

auto code_listener_nop::operator()(const jmp &op) -> status {
  (void)op;

  return {};
}

auto code_listener_nop::operator()(const end &op) -> status {
  (void)op;

  return {};
}

auto code_listener_nop::operator()(const ret &op) -> status {
  (void)op;

  return {};
}

auto code_listener_nop::operator()(const drop &op) -> status {
  (void)op;

  return {};
}

auto code_listener_nop::operator()(const dup &op) -> status {
  (void)op;

  return {};
}

auto code_listener_nop::operator()(const swap &op) -> status {
  (void)op;

  return {};
}

auto code_listener_nop::operator()(const elem &op) -> status {
  (void)op;

  return {};
}

auto code_listener_nop::operator()(const ld_glo &op) -> status {
  (void)op;

  return {};
}

auto code_listener_nop::operator()(const ld_loc &op) -> status {
  (void)op;

  return {};
}

auto code_listener_nop::operator()(const ld_arg &op) -> status {
  (void)op;

  return {};
}

auto code_listener_nop::operator()(const ld_clo &op) -> status {
  (void)op;

  return {};
}

auto code_listener_nop::operator()(const lda_glo &op) -> status {
  (void)op;

  return {};
}

auto code_listener_nop::operator()(const lda_loc &op) -> status {
  (void)op;

  return {};
}

auto code_listener_nop::operator()(const lda_arg &op) -> status {
  (void)op;

  return {};
}

auto code_listener_nop::operator()(const lda_clo &op) -> status {
  (void)op;

  return {};
}

auto code_listener_nop::operator()(const st_glo &op) -> status {
  (void)op;

  return {};
}

auto code_listener_nop::operator()(const st_loc &op) -> status {
  (void)op;

  return {};
}

auto code_listener_nop::operator()(const st_arg &op) -> status {
  (void)op;

  return {};
}

auto code_listener_nop::operator()(const st_clo &op) -> status {
  (void)op;

  return {};
}

auto code_listener_nop::operator()(const cjmp_z &op) -> status {
  (void)op;

  return {};
}

auto code_listener_nop::operator()(const cjmp_nz &op) -> status {
  (void)op;

  return {};
}

auto code_listener_nop::operator()(const begin &op) -> status {
  (void)op;

  return {};
}

auto code_listener_nop::operator()(const begin_closure &op) -> status {
  (void)op;

  return {};
}

auto code_listener_nop::operator()(const closure &op, std::span<const closure::capture> captures)
  -> status {
  (void)op;
  (void)captures;

  return {};
}

auto code_listener_nop::operator()(const callc &op) -> status {
  (void)op;

  return {};
}

auto code_listener_nop::operator()(const call &op) -> status {
  (void)op;

  return {};
}

auto code_listener_nop::operator()(const tag &op) -> status {
  (void)op;

  return {};
}

auto code_listener_nop::operator()(const array &op) -> status {
  (void)op;

  return {};
}

auto code_listener_nop::operator()(const fail &op) -> status {
  (void)op;

  return {};
}

auto code_listener_nop::operator()(const fail_keep &op) -> status {
  (void)op;

  return {};
}

auto code_listener_nop::operator()(const line &op) -> status {
  (void)op;

  return {};
}

auto code_listener_nop::operator()(const patt_str_cmp &op) -> status {
  (void)op;

  return {};
}

auto code_listener_nop::operator()(const patt_string &op) -> status {
  (void)op;

  return {};
}

auto code_listener_nop::operator()(const patt_array &op) -> status {
  (void)op;

  return {};
}

auto code_listener_nop::operator()(const patt_sexp &op) -> status {
  (void)op;

  return {};
}

auto code_listener_nop::operator()(const patt_boxed &op) -> status {
  (void)op;

  return {};
}

auto code_listener_nop::operator()(const patt_unboxed &op) -> status {
  (void)op;

  return {};
}

auto code_listener_nop::operator()(const patt_closure &op) -> status {
  (void)op;

  return {};
}

auto code_listener_nop::operator()(const barray &op) -> status {
  (void)op;

  return {};
}

auto code_listener_nop::operator()(const eof &op) -> status {
  (void)op;

  return {};
}

} // namespace lamavm::bc
