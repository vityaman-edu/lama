#pragma once

#include "core/exception.hpp"
#include "layout.hpp"

#include <span>

namespace lamavm::bc {

class code_listener {
public:
  virtual ~code_listener() = 0;

  virtual auto operator()(const add &op) -> status = 0;
  virtual auto operator()(const sub &op) -> status = 0;
  virtual auto operator()(const mul &op) -> status = 0;
  virtual auto operator()(const div &op) -> status = 0;
  virtual auto operator()(const mod &op) -> status = 0;
  virtual auto operator()(const lt &op) -> status = 0;
  virtual auto operator()(const le &op) -> status = 0;
  virtual auto operator()(const gt &op) -> status = 0;
  virtual auto operator()(const ge &op) -> status = 0;
  virtual auto operator()(const eq &op) -> status = 0;
  virtual auto operator()(const ne &op) -> status = 0;
  virtual auto operator()(const logical_and &op) -> status = 0;
  virtual auto operator()(const logical_or &op) -> status = 0;
  virtual auto operator()(const constant &op) -> status = 0;
  virtual auto operator()(const string &op) -> status = 0;
  virtual auto operator()(const sexp &op) -> status = 0;
  virtual auto operator()(const sti &op) -> status = 0;
  virtual auto operator()(const sta &op) -> status = 0;
  virtual auto operator()(const jmp &op) -> status = 0;
  virtual auto operator()(const end &op) -> status = 0;
  virtual auto operator()(const ret &op) -> status = 0;
  virtual auto operator()(const drop &op) -> status = 0;
  virtual auto operator()(const dup &op) -> status = 0;
  virtual auto operator()(const swap &op) -> status = 0;
  virtual auto operator()(const elem &op) -> status = 0;
  virtual auto operator()(const ld_glo &op) -> status = 0;
  virtual auto operator()(const ld_loc &op) -> status = 0;
  virtual auto operator()(const ld_arg &op) -> status = 0;
  virtual auto operator()(const ld_clo &op) -> status = 0;
  virtual auto operator()(const lda_glo &op) -> status = 0;
  virtual auto operator()(const lda_loc &op) -> status = 0;
  virtual auto operator()(const lda_arg &op) -> status = 0;
  virtual auto operator()(const lda_clo &op) -> status = 0;
  virtual auto operator()(const st_glo &op) -> status = 0;
  virtual auto operator()(const st_loc &op) -> status = 0;
  virtual auto operator()(const st_arg &op) -> status = 0;
  virtual auto operator()(const st_clo &op) -> status = 0;
  virtual auto operator()(const cjmp_z &op) -> status = 0;
  virtual auto operator()(const cjmp_nz &op) -> status = 0;
  virtual auto operator()(const begin &op) -> status = 0;
  virtual auto operator()(const begin_closure &op) -> status = 0;
  virtual auto operator()(const closure &op, std::span<const closure::capture> captures)
    -> status = 0;
  virtual auto operator()(const callc &op) -> status = 0;
  virtual auto operator()(const call &op) -> status = 0;
  virtual auto operator()(const tag &op) -> status = 0;
  virtual auto operator()(const array &op) -> status = 0;
  virtual auto operator()(const fail &op) -> status = 0;
  virtual auto operator()(const fail_keep &op) -> status = 0;
  virtual auto operator()(const line &op) -> status = 0;
  virtual auto operator()(const patt_str_cmp &op) -> status = 0;
  virtual auto operator()(const patt_string &op) -> status = 0;
  virtual auto operator()(const patt_array &op) -> status = 0;
  virtual auto operator()(const patt_sexp &op) -> status = 0;
  virtual auto operator()(const patt_boxed &op) -> status = 0;
  virtual auto operator()(const patt_unboxed &op) -> status = 0;
  virtual auto operator()(const patt_closure &op) -> status = 0;
  virtual auto operator()(const barray &op) -> status = 0;
  virtual auto operator()(const eof &op) -> status = 0;
};

} // namespace lamavm::bc
