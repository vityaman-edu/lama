#pragma once

#include "bc/code_listener.hpp"
#include "bc/file_listener.hpp"
#include "bc/layout.hpp"
#include "core/exception.hpp"

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

  auto operator()(const header &op) -> status override;
  auto operator()(std::string_view table) -> status override;
  auto operator()(std::span<const std::byte> bytes)
    -> std::expected<void, std::vector<error>> override;
  auto operator()(const import &op) -> status override;
  auto operator()(const public_symbol &op) -> status override;
  auto operator()(const add &op) -> status override;
  auto operator()(const sub &op) -> status override;
  auto operator()(const mul &op) -> status override;
  auto operator()(const div &op) -> status override;
  auto operator()(const mod &op) -> status override;
  auto operator()(const lt &op) -> status override;
  auto operator()(const le &op) -> status override;
  auto operator()(const gt &op) -> status override;
  auto operator()(const ge &op) -> status override;
  auto operator()(const eq &op) -> status override;
  auto operator()(const ne &op) -> status override;
  auto operator()(const logical_and &op) -> status override;
  auto operator()(const logical_or &op) -> status override;
  auto operator()(const constant &op) -> status override;
  auto operator()(const string &op) -> status override;
  auto operator()(const sexp &op) -> status override;
  auto operator()(const sti &op) -> status override;
  auto operator()(const sta &op) -> status override;
  auto operator()(const jmp &op) -> status override;
  auto operator()(const end &op) -> status override;
  auto operator()(const ret &op) -> status override;
  auto operator()(const drop &op) -> status override;
  auto operator()(const dup &op) -> status override;
  auto operator()(const swap &op) -> status override;
  auto operator()(const elem &op) -> status override;
  auto operator()(const ld_glo &op) -> status override;
  auto operator()(const ld_loc &op) -> status override;
  auto operator()(const ld_arg &op) -> status override;
  auto operator()(const ld_clo &op) -> status override;
  auto operator()(const lda_glo &op) -> status override;
  auto operator()(const lda_loc &op) -> status override;
  auto operator()(const lda_arg &op) -> status override;
  auto operator()(const lda_clo &op) -> status override;
  auto operator()(const st_glo &op) -> status override;
  auto operator()(const st_loc &op) -> status override;
  auto operator()(const st_arg &op) -> status override;
  auto operator()(const st_clo &op) -> status override;
  auto operator()(const cjmp_z &op) -> status override;
  auto operator()(const cjmp_nz &op) -> status override;
  auto operator()(const begin &op) -> status override;
  auto operator()(const begin_closure &op) -> status override;
  auto operator()(const closure &op, std::span<const closure::capture> captures) -> status override;
  auto operator()(const callc &op) -> status override;
  auto operator()(const call &op) -> status override;
  auto operator()(const tag &op) -> status override;
  auto operator()(const array &op) -> status override;
  auto operator()(const fail &op) -> status override;
  auto operator()(const fail_keep &op) -> status override;
  auto operator()(const line &op) -> status override;
  auto operator()(const patt_str_cmp &op) -> status override;
  auto operator()(const patt_string &op) -> status override;
  auto operator()(const patt_array &op) -> status override;
  auto operator()(const patt_sexp &op) -> status override;
  auto operator()(const patt_boxed &op) -> status override;
  auto operator()(const patt_unboxed &op) -> status override;
  auto operator()(const patt_closure &op) -> status override;
  auto operator()(const barray &op) -> status override;
  auto operator()(const eof &op) -> status override;

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
