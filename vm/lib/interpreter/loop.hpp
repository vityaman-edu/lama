#pragma once

#include "bc/code_listener.hpp"
#include "bc/layout.hpp"
#include "core/exception.hpp"
#include "state.hpp"

#include <cstddef>
#include <cstdint>
#include <span>
#include <string>

namespace lamavm::interpreter {

class loop : public bc::code_listener, private state {
public:
  loop(std::string string_table, std::span<const std::byte> code);

  auto run() -> status;

  using state::stack_top;

  using bc::code_listener::operator();

  auto operator()(const bc::add &op) -> status override;
  auto operator()(const bc::sub &op) -> status override;
  auto operator()(const bc::mul &op) -> status override;
  auto operator()(const bc::div &op) -> status override;
  auto operator()(const bc::mod &op) -> status override;
  auto operator()(const bc::lt &op) -> status override;
  auto operator()(const bc::le &op) -> status override;
  auto operator()(const bc::gt &op) -> status override;
  auto operator()(const bc::ge &op) -> status override;
  auto operator()(const bc::eq &op) -> status override;
  auto operator()(const bc::ne &op) -> status override;
  auto operator()(const bc::logical_and &op) -> status override;
  auto operator()(const bc::logical_or &op) -> status override;
  auto operator()(const bc::constant &op) -> status override;
  auto operator()(const bc::string &op) -> status override;
  auto operator()(const bc::sexp &op) -> status override;
  auto operator()(const bc::sti &op) -> status override;
  auto operator()(const bc::sta &op) -> status override;
  auto operator()(const bc::jmp &op) -> status override;
  auto operator()(const bc::end &op) -> status override;
  auto operator()(const bc::ret &op) -> status override;
  auto operator()(const bc::drop &op) -> status override;
  auto operator()(const bc::dup &op) -> status override;
  auto operator()(const bc::swap &op) -> status override;
  auto operator()(const bc::elem &op) -> status override;
  auto operator()(const bc::ld_glo &op) -> status override;
  auto operator()(const bc::ld_loc &op) -> status override;
  auto operator()(const bc::ld_arg &op) -> status override;
  auto operator()(const bc::ld_clo &op) -> status override;
  auto operator()(const bc::lda_glo &op) -> status override;
  auto operator()(const bc::lda_loc &op) -> status override;
  auto operator()(const bc::lda_arg &op) -> status override;
  auto operator()(const bc::lda_clo &op) -> status override;
  auto operator()(const bc::st_glo &op) -> status override;
  auto operator()(const bc::st_loc &op) -> status override;
  auto operator()(const bc::st_arg &op) -> status override;
  auto operator()(const bc::st_clo &op) -> status override;
  auto operator()(const bc::cjmp_z &op) -> status override;
  auto operator()(const bc::cjmp_nz &op) -> status override;
  auto operator()(const bc::begin &op) -> status override;
  auto operator()(const bc::begin_closure &op) -> status override;
  auto operator()(const bc::closure &op, std::span<const bc::closure::capture> captures)
    -> status override;
  auto operator()(const bc::callc &op) -> status override;
  auto operator()(const bc::call &op) -> status override;
  auto operator()(const bc::tag &op) -> status override;
  auto operator()(const bc::array &op) -> status override;
  auto operator()(const bc::fail &op) -> status override;
  auto operator()(const bc::fail_keep &op) -> status override;
  auto operator()(const bc::line &op) -> status override;
  auto operator()(const bc::patt_str_cmp &op) -> status override;
  auto operator()(const bc::patt_string &op) -> status override;
  auto operator()(const bc::patt_array &op) -> status override;
  auto operator()(const bc::patt_sexp &op) -> status override;
  auto operator()(const bc::patt_boxed &op) -> status override;
  auto operator()(const bc::patt_unboxed &op) -> status override;
  auto operator()(const bc::patt_closure &op) -> status override;
  auto operator()(const bc::barray &op) -> status override;
  auto operator()(const bc::eof &op) -> status override;

private:
  template <std::int64_t (*function)(void *, void *)>
  auto binop() -> status;
};

} // namespace lamavm::interpreter
