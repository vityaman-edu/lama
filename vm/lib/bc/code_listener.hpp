#pragma once

#include "layout.hpp"

#include <cstddef>
#include <span>
#include <string_view>

namespace lamavm::bc {

class code_listener {
public:
  virtual ~code_listener() = 0;

  virtual void operator()(std::size_t offset, std::string_view message) = 0;
  virtual void operator()(const add &instruction) = 0;
  virtual void operator()(const sub &instruction) = 0;
  virtual void operator()(const mul &instruction) = 0;
  virtual void operator()(const div &instruction) = 0;
  virtual void operator()(const mod &instruction) = 0;
  virtual void operator()(const lt &instruction) = 0;
  virtual void operator()(const le &instruction) = 0;
  virtual void operator()(const gt &instruction) = 0;
  virtual void operator()(const ge &instruction) = 0;
  virtual void operator()(const eq &instruction) = 0;
  virtual void operator()(const ne &instruction) = 0;
  virtual void operator()(const logical_and &instruction) = 0;
  virtual void operator()(const logical_or &instruction) = 0;
  virtual void operator()(const constant &instruction) = 0;
  virtual void operator()(const string &instruction) = 0;
  virtual void operator()(const sexp &instruction) = 0;
  virtual void operator()(const sti &instruction) = 0;
  virtual void operator()(const sta &instruction) = 0;
  virtual void operator()(const jmp &instruction) = 0;
  virtual void operator()(const end &instruction) = 0;
  virtual void operator()(const ret &instruction) = 0;
  virtual void operator()(const drop &instruction) = 0;
  virtual void operator()(const dup &instruction) = 0;
  virtual void operator()(const swap &instruction) = 0;
  virtual void operator()(const elem &instruction) = 0;
  virtual void operator()(const ld_glo &instruction) = 0;
  virtual void operator()(const ld_loc &instruction) = 0;
  virtual void operator()(const ld_arg &instruction) = 0;
  virtual void operator()(const ld_clo &instruction) = 0;
  virtual void operator()(const lda_glo &instruction) = 0;
  virtual void operator()(const lda_loc &instruction) = 0;
  virtual void operator()(const lda_arg &instruction) = 0;
  virtual void operator()(const lda_clo &instruction) = 0;
  virtual void operator()(const st_glo &instruction) = 0;
  virtual void operator()(const st_loc &instruction) = 0;
  virtual void operator()(const st_arg &instruction) = 0;
  virtual void operator()(const st_clo &instruction) = 0;
  virtual void operator()(const cjmp_z &instruction) = 0;
  virtual void operator()(const cjmp_nz &instruction) = 0;
  virtual void operator()(const begin &instruction) = 0;
  virtual void operator()(const begin_closure &instruction) = 0;
  virtual void
  operator()(const closure &instruction, std::span<const closure::capture> captures) = 0;
  virtual void operator()(const callc &instruction) = 0;
  virtual void operator()(const call &instruction) = 0;
  virtual void operator()(const tag &instruction) = 0;
  virtual void operator()(const array &instruction) = 0;
  virtual void operator()(const fail &instruction) = 0;
  virtual void operator()(const fail_keep &instruction) = 0;
  virtual void operator()(const line &instruction) = 0;
  virtual void operator()(const patt_str_cmp &instruction) = 0;
  virtual void operator()(const patt_string &instruction) = 0;
  virtual void operator()(const patt_array &instruction) = 0;
  virtual void operator()(const patt_sexp &instruction) = 0;
  virtual void operator()(const patt_boxed &instruction) = 0;
  virtual void operator()(const patt_unboxed &instruction) = 0;
  virtual void operator()(const patt_closure &instruction) = 0;
  virtual void operator()(const barray &instruction) = 0;
  virtual void operator()(const eof &instruction) = 0;
};

} // namespace lamavm::bc
