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
#include <vector>

namespace lamavm::bc::disassembly {

class legacy : public file_listener, public code_listener {
public:
  explicit legacy(std::ostream &output);

  using file_listener::operator();
  using code_listener::operator();

  void operator()(const header &op) override;
  void operator()(std::string_view table) override;
  auto operator()(std::span<const std::byte> bytes)
    -> std::expected<void, std::vector<std::string>> override;
  void operator()(const import &op) override;
  void operator()(const public_symbol &op) override;
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

private:
  [[nodiscard]] auto string_at(std::int32_t offset) const -> std::string_view;
  [[nodiscard]] auto target(std::int32_t value) const -> std::string;
  [[nodiscard]] auto designation(enum closure::capture::kind kind, std::int32_t index) const
    -> std::string;
  void emit(std::size_t size, std::string_view op);

  std::ostream *out;
  std::string strings;
  std::size_t code_offset = 0;
};

} // namespace lamavm::bc::disassembly
