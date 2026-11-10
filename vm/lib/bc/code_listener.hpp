#pragma once

#include "layout.hpp"

#include <span>

namespace lamavm::bc {

class code_listener {
public:
  virtual ~code_listener() = 0;

  virtual void operator()(const add &op) = 0;
  virtual void operator()(const sub &op) = 0;
  virtual void operator()(const mul &op) = 0;
  virtual void operator()(const div &op) = 0;
  virtual void operator()(const mod &op) = 0;
  virtual void operator()(const lt &op) = 0;
  virtual void operator()(const le &op) = 0;
  virtual void operator()(const gt &op) = 0;
  virtual void operator()(const ge &op) = 0;
  virtual void operator()(const eq &op) = 0;
  virtual void operator()(const ne &op) = 0;
  virtual void operator()(const logical_and &op) = 0;
  virtual void operator()(const logical_or &op) = 0;
  virtual void operator()(const constant &op) = 0;
  virtual void operator()(const string &op) = 0;
  virtual void operator()(const sexp &op) = 0;
  virtual void operator()(const sti &op) = 0;
  virtual void operator()(const sta &op) = 0;
  virtual void operator()(const jmp &op) = 0;
  virtual void operator()(const end &op) = 0;
  virtual void operator()(const ret &op) = 0;
  virtual void operator()(const drop &op) = 0;
  virtual void operator()(const dup &op) = 0;
  virtual void operator()(const swap &op) = 0;
  virtual void operator()(const elem &op) = 0;
  virtual void operator()(const ld_glo &op) = 0;
  virtual void operator()(const ld_loc &op) = 0;
  virtual void operator()(const ld_arg &op) = 0;
  virtual void operator()(const ld_clo &op) = 0;
  virtual void operator()(const lda_glo &op) = 0;
  virtual void operator()(const lda_loc &op) = 0;
  virtual void operator()(const lda_arg &op) = 0;
  virtual void operator()(const lda_clo &op) = 0;
  virtual void operator()(const st_glo &op) = 0;
  virtual void operator()(const st_loc &op) = 0;
  virtual void operator()(const st_arg &op) = 0;
  virtual void operator()(const st_clo &op) = 0;
  virtual void operator()(const cjmp_z &op) = 0;
  virtual void operator()(const cjmp_nz &op) = 0;
  virtual void operator()(const begin &op) = 0;
  virtual void operator()(const begin_closure &op) = 0;
  virtual void operator()(const closure &op, std::span<const closure::capture> captures) = 0;
  virtual void operator()(const callc &op) = 0;
  virtual void operator()(const call &op) = 0;
  virtual void operator()(const tag &op) = 0;
  virtual void operator()(const array &op) = 0;
  virtual void operator()(const fail &op) = 0;
  virtual void operator()(const fail_keep &op) = 0;
  virtual void operator()(const line &op) = 0;
  virtual void operator()(const patt_str_cmp &op) = 0;
  virtual void operator()(const patt_string &op) = 0;
  virtual void operator()(const patt_array &op) = 0;
  virtual void operator()(const patt_sexp &op) = 0;
  virtual void operator()(const patt_boxed &op) = 0;
  virtual void operator()(const patt_unboxed &op) = 0;
  virtual void operator()(const patt_closure &op) = 0;
  virtual void operator()(const barray &op) = 0;
  virtual void operator()(const eof &op) = 0;
};

} // namespace lamavm::bc
