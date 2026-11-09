#pragma once

#include "layout.hpp"

#include <cstddef>
#include <span>
#include <string_view>

namespace lamavm::bc {

class base_listener {
public:
  base_listener() = default;
  base_listener(const base_listener &) = default;
  base_listener(base_listener &&) = delete;
  auto operator=(const base_listener &) -> base_listener & = default;
  auto operator=(base_listener &&) -> base_listener & = delete;
  virtual ~base_listener() = default;

  virtual void operator()(std::size_t offset, std::string_view message) {
    (void)offset;
    (void)message;
  }

  virtual void operator()(const header &instruction) {
    (void)instruction;
  }

  virtual void operator()(std::string_view strings) {
    (void)strings;
  }

  virtual void operator()(const import &instruction) {
    (void)instruction;
  }

  virtual void operator()(const public_symbol &instruction) {
    (void)instruction;
  }

  virtual void operator()(const add &instruction) {
    (void)instruction;
  }

  virtual void operator()(const sub &instruction) {
    (void)instruction;
  }

  virtual void operator()(const mul &instruction) {
    (void)instruction;
  }

  virtual void operator()(const div &instruction) {
    (void)instruction;
  }

  virtual void operator()(const mod &instruction) {
    (void)instruction;
  }

  virtual void operator()(const lt &instruction) {
    (void)instruction;
  }

  virtual void operator()(const le &instruction) {
    (void)instruction;
  }

  virtual void operator()(const gt &instruction) {
    (void)instruction;
  }

  virtual void operator()(const ge &instruction) {
    (void)instruction;
  }

  virtual void operator()(const eq &instruction) {
    (void)instruction;
  }

  virtual void operator()(const ne &instruction) {
    (void)instruction;
  }

  virtual void operator()(const logical_and &instruction) {
    (void)instruction;
  }

  virtual void operator()(const logical_or &instruction) {
    (void)instruction;
  }

  virtual void operator()(const constant &instruction) {
    (void)instruction;
  }

  virtual void operator()(const string &instruction) {
    (void)instruction;
  }

  virtual void operator()(const sexp &instruction) {
    (void)instruction;
  }

  virtual void operator()(const sti &instruction) {
    (void)instruction;
  }

  virtual void operator()(const sta &instruction) {
    (void)instruction;
  }

  virtual void operator()(const jmp &instruction) {
    (void)instruction;
  }

  virtual void operator()(const end &instruction) {
    (void)instruction;
  }

  virtual void operator()(const ret &instruction) {
    (void)instruction;
  }

  virtual void operator()(const drop &instruction) {
    (void)instruction;
  }

  virtual void operator()(const dup &instruction) {
    (void)instruction;
  }

  virtual void operator()(const swap &instruction) {
    (void)instruction;
  }

  virtual void operator()(const elem &instruction) {
    (void)instruction;
  }

  virtual void operator()(const ld_glo &instruction) {
    (void)instruction;
  }

  virtual void operator()(const ld_loc &instruction) {
    (void)instruction;
  }

  virtual void operator()(const ld_arg &instruction) {
    (void)instruction;
  }

  virtual void operator()(const ld_clo &instruction) {
    (void)instruction;
  }

  virtual void operator()(const lda_glo &instruction) {
    (void)instruction;
  }

  virtual void operator()(const lda_loc &instruction) {
    (void)instruction;
  }

  virtual void operator()(const lda_arg &instruction) {
    (void)instruction;
  }

  virtual void operator()(const lda_clo &instruction) {
    (void)instruction;
  }

  virtual void operator()(const st_glo &instruction) {
    (void)instruction;
  }

  virtual void operator()(const st_loc &instruction) {
    (void)instruction;
  }

  virtual void operator()(const st_arg &instruction) {
    (void)instruction;
  }

  virtual void operator()(const st_clo &instruction) {
    (void)instruction;
  }

  virtual void operator()(const cjmp_z &instruction) {
    (void)instruction;
  }

  virtual void operator()(const cjmp_nz &instruction) {
    (void)instruction;
  }

  virtual void operator()(const begin &instruction) {
    (void)instruction;
  }

  virtual void operator()(const begin_closure &instruction) {
    (void)instruction;
  }

  virtual void operator()(const closure &instruction, std::span<const closure::capture> captures) {
    (void)instruction;
    (void)captures;
  }

  virtual void operator()(const callc &instruction) {
    (void)instruction;
  }

  virtual void operator()(const call &instruction) {
    (void)instruction;
  }

  virtual void operator()(const tag &instruction) {
    (void)instruction;
  }

  virtual void operator()(const array &instruction) {
    (void)instruction;
  }

  virtual void operator()(const fail &instruction) {
    (void)instruction;
  }

  virtual void operator()(const fail_keep &instruction) {
    (void)instruction;
  }

  virtual void operator()(const line &instruction) {
    (void)instruction;
  }

  virtual void operator()(const patt_str_cmp &instruction) {
    (void)instruction;
  }

  virtual void operator()(const patt_string &instruction) {
    (void)instruction;
  }

  virtual void operator()(const patt_array &instruction) {
    (void)instruction;
  }

  virtual void operator()(const patt_sexp &instruction) {
    (void)instruction;
  }

  virtual void operator()(const patt_boxed &instruction) {
    (void)instruction;
  }

  virtual void operator()(const patt_unboxed &instruction) {
    (void)instruction;
  }

  virtual void operator()(const patt_closure &instruction) {
    (void)instruction;
  }

  virtual void operator()(const barray &instruction) {
    (void)instruction;
  }

  virtual void operator()(const eof &instruction) {
    (void)instruction;
  }
};

} // namespace lamavm::bc
