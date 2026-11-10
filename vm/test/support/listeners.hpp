#pragma once

#include "bc/code_listener.hpp"
#include "bc/file_listener.hpp"
#include "bc/layout.hpp"

#include <cstddef>
#include <expected>
#include <span>
#include <string_view>
#include <variant>

namespace lamavm::bc::test {

class noop_file_listener : public file_listener {
public:
  using file_listener::operator();

  void operator()(std::size_t offset, std::string_view message) override;
  void operator()(const header &instruction) override;
  void operator()(const import &instruction) override;
  void operator()(const public_symbol &instruction) override;
  void operator()(std::string_view strings) override;
  auto operator()(std::span<const std::byte> code) -> std::expected<void, std::monostate> override;
};

class noop_code_listener : public code_listener {
public:
  using code_listener::operator();

  void operator()(std::size_t offset, std::string_view message) override;
  void operator()(const add &instruction) override;
  void operator()(const sub &instruction) override;
  void operator()(const mul &instruction) override;
  void operator()(const div &instruction) override;
  void operator()(const mod &instruction) override;
  void operator()(const lt &instruction) override;
  void operator()(const le &instruction) override;
  void operator()(const gt &instruction) override;
  void operator()(const ge &instruction) override;
  void operator()(const eq &instruction) override;
  void operator()(const ne &instruction) override;
  void operator()(const logical_and &instruction) override;
  void operator()(const logical_or &instruction) override;
  void operator()(const constant &instruction) override;
  void operator()(const string &instruction) override;
  void operator()(const sexp &instruction) override;
  void operator()(const sti &instruction) override;
  void operator()(const sta &instruction) override;
  void operator()(const jmp &instruction) override;
  void operator()(const end &instruction) override;
  void operator()(const ret &instruction) override;
  void operator()(const drop &instruction) override;
  void operator()(const dup &instruction) override;
  void operator()(const swap &instruction) override;
  void operator()(const elem &instruction) override;
  void operator()(const ld_glo &instruction) override;
  void operator()(const ld_loc &instruction) override;
  void operator()(const ld_arg &instruction) override;
  void operator()(const ld_clo &instruction) override;
  void operator()(const lda_glo &instruction) override;
  void operator()(const lda_loc &instruction) override;
  void operator()(const lda_arg &instruction) override;
  void operator()(const lda_clo &instruction) override;
  void operator()(const st_glo &instruction) override;
  void operator()(const st_loc &instruction) override;
  void operator()(const st_arg &instruction) override;
  void operator()(const st_clo &instruction) override;
  void operator()(const cjmp_z &instruction) override;
  void operator()(const cjmp_nz &instruction) override;
  void operator()(const begin &instruction) override;
  void operator()(const begin_closure &instruction) override;
  void operator()(const closure &instruction, std::span<const closure::capture> captures) override;
  void operator()(const callc &instruction) override;
  void operator()(const call &instruction) override;
  void operator()(const tag &instruction) override;
  void operator()(const array &instruction) override;
  void operator()(const fail &instruction) override;
  void operator()(const fail_keep &instruction) override;
  void operator()(const line &instruction) override;
  void operator()(const patt_str_cmp &instruction) override;
  void operator()(const patt_string &instruction) override;
  void operator()(const patt_array &instruction) override;
  void operator()(const patt_sexp &instruction) override;
  void operator()(const patt_boxed &instruction) override;
  void operator()(const patt_unboxed &instruction) override;
  void operator()(const patt_closure &instruction) override;
  void operator()(const barray &instruction) override;
  void operator()(const eof &instruction) override;
};

} // namespace lamavm::bc::test
