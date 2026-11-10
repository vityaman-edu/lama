#pragma once

#include "bc/code_listener.hpp"
#include "bc/file_listener.hpp"
#include "bc/layout.hpp"

#include <cstddef>
#include <cstdint>
#include <expected>
#include <iosfwd>
#include <span>
#include <string>
#include <string_view>
#include <variant>

namespace lamavm::bc::disassembly {

class legacy : public file_listener, public code_listener {
public:
  legacy(std::ostream &output, std::ostream &error_output);

  using file_listener::operator();
  using code_listener::operator();

  void operator()(std::size_t offset, std::string_view message) override;

  void operator()(const header &instruction) override;
  void operator()(std::string_view table) override;
  auto operator()(std::span<const std::byte> bytes) -> std::expected<void, std::monostate> override;
  void operator()(const import &instruction) override;
  void operator()(const public_symbol &instruction) override;
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

private:
  [[nodiscard]] auto string_at(std::int32_t offset) const -> std::string_view;
  [[nodiscard]] auto target(std::int32_t value) const -> std::string;
  [[nodiscard]] auto designation(enum closure::capture::kind kind, std::int32_t index) const
    -> std::string;
  void emit(std::size_t size, std::string_view instruction);

  std::ostream *out;
  std::ostream *err;
  std::string strings;
  std::size_t code_offset = 0;
};

} // namespace lamavm::bc::disassembly
