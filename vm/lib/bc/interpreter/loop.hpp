#pragma once

#include "bc/code_listener.hpp"
#include "bc/layout.hpp"

#include <span>

namespace lamavm::bc::interpreter {

class loop : public code_listener {
public:
  using code_listener::operator();

  void operator()(const add &op) override;
  void operator()(const sub &op) override;
  void operator()(const mul &op) override;
  void operator()(const div &op) override;
  void operator()(const mod &op) override;
  void operator()(const lt &op) override;
  void operator()(const le &op) override;
  void operator()(const gt &op) override;
  void operator()(const ge &op) override;
  void operator()(const eq &op) override;
  void operator()(const ne &op) override;
  void operator()(const logical_and &op) override;
  void operator()(const logical_or &op) override;
  void operator()(const constant &op) override;
  void operator()(const string &op) override;
  void operator()(const sexp &op) override;
  void operator()(const sti &op) override;
  void operator()(const sta &op) override;
  void operator()(const jmp &op) override;
  void operator()(const end &op) override;
  void operator()(const ret &op) override;
  void operator()(const drop &op) override;
  void operator()(const dup &op) override;
  void operator()(const swap &op) override;
  void operator()(const elem &op) override;
  void operator()(const ld_glo &op) override;
  void operator()(const ld_loc &op) override;
  void operator()(const ld_arg &op) override;
  void operator()(const ld_clo &op) override;
  void operator()(const lda_glo &op) override;
  void operator()(const lda_loc &op) override;
  void operator()(const lda_arg &op) override;
  void operator()(const lda_clo &op) override;
  void operator()(const st_glo &op) override;
  void operator()(const st_loc &op) override;
  void operator()(const st_arg &op) override;
  void operator()(const st_clo &op) override;
  void operator()(const cjmp_z &op) override;
  void operator()(const cjmp_nz &op) override;
  void operator()(const begin &op) override;
  void operator()(const begin_closure &op) override;
  void operator()(const closure &op, std::span<const closure::capture> captures) override;
  void operator()(const callc &op) override;
  void operator()(const call &op) override;
  void operator()(const tag &op) override;
  void operator()(const array &op) override;
  void operator()(const fail &op) override;
  void operator()(const fail_keep &op) override;
  void operator()(const line &op) override;
  void operator()(const patt_str_cmp &op) override;
  void operator()(const patt_string &op) override;
  void operator()(const patt_array &op) override;
  void operator()(const patt_sexp &op) override;
  void operator()(const patt_boxed &op) override;
  void operator()(const patt_unboxed &op) override;
  void operator()(const patt_closure &op) override;
  void operator()(const barray &op) override;
  void operator()(const eof &op) override;
};

} // namespace lamavm::bc::interpreter
