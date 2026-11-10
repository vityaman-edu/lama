#pragma once

#include "bc/code_listener.hpp"
#include "bc/layout.hpp"

#include <span>

namespace lamavm::interpreter {

class loop : public bc::code_listener {
public:
  using bc::code_listener::operator();

  void operator()(const bc::add &op) override;
  void operator()(const bc::sub &op) override;
  void operator()(const bc::mul &op) override;
  void operator()(const bc::div &op) override;
  void operator()(const bc::mod &op) override;
  void operator()(const bc::lt &op) override;
  void operator()(const bc::le &op) override;
  void operator()(const bc::gt &op) override;
  void operator()(const bc::ge &op) override;
  void operator()(const bc::eq &op) override;
  void operator()(const bc::ne &op) override;
  void operator()(const bc::logical_and &op) override;
  void operator()(const bc::logical_or &op) override;
  void operator()(const bc::constant &op) override;
  void operator()(const bc::string &op) override;
  void operator()(const bc::sexp &op) override;
  void operator()(const bc::sti &op) override;
  void operator()(const bc::sta &op) override;
  void operator()(const bc::jmp &op) override;
  void operator()(const bc::end &op) override;
  void operator()(const bc::ret &op) override;
  void operator()(const bc::drop &op) override;
  void operator()(const bc::dup &op) override;
  void operator()(const bc::swap &op) override;
  void operator()(const bc::elem &op) override;
  void operator()(const bc::ld_glo &op) override;
  void operator()(const bc::ld_loc &op) override;
  void operator()(const bc::ld_arg &op) override;
  void operator()(const bc::ld_clo &op) override;
  void operator()(const bc::lda_glo &op) override;
  void operator()(const bc::lda_loc &op) override;
  void operator()(const bc::lda_arg &op) override;
  void operator()(const bc::lda_clo &op) override;
  void operator()(const bc::st_glo &op) override;
  void operator()(const bc::st_loc &op) override;
  void operator()(const bc::st_arg &op) override;
  void operator()(const bc::st_clo &op) override;
  void operator()(const bc::cjmp_z &op) override;
  void operator()(const bc::cjmp_nz &op) override;
  void operator()(const bc::begin &op) override;
  void operator()(const bc::begin_closure &op) override;
  void operator()(const bc::closure &op, std::span<const bc::closure::capture> captures) override;
  void operator()(const bc::callc &op) override;
  void operator()(const bc::call &op) override;
  void operator()(const bc::tag &op) override;
  void operator()(const bc::array &op) override;
  void operator()(const bc::fail &op) override;
  void operator()(const bc::fail_keep &op) override;
  void operator()(const bc::line &op) override;
  void operator()(const bc::patt_str_cmp &op) override;
  void operator()(const bc::patt_string &op) override;
  void operator()(const bc::patt_array &op) override;
  void operator()(const bc::patt_sexp &op) override;
  void operator()(const bc::patt_boxed &op) override;
  void operator()(const bc::patt_unboxed &op) override;
  void operator()(const bc::patt_closure &op) override;
  void operator()(const bc::barray &op) override;
  void operator()(const bc::eof &op) override;
};

} // namespace lamavm::interpreter
