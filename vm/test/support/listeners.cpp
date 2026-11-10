#include "listeners.hpp"

#include "bc/layout.hpp"

#include <cstddef>
#include <expected>
#include <span>
#include <string_view>
#include <variant>

namespace lamavm::bc::test {

void noop_file_listener::operator()(std::size_t offset, std::string_view message) {
  (void)offset;
  (void)message;
}

void noop_file_listener::operator()(const header &instruction) {
  (void)instruction;
}

void noop_file_listener::operator()(const import &instruction) {
  (void)instruction;
}

void noop_file_listener::operator()(const public_symbol &instruction) {
  (void)instruction;
}

void noop_file_listener::operator()(std::string_view strings) {
  (void)strings;
}

auto noop_file_listener::operator()(std::span<const std::byte> code)
  -> std::expected<void, std::monostate> {
  (void)code;

  return {};
}

void noop_code_listener::operator()(std::size_t offset, std::string_view message) {
  (void)offset;
  (void)message;
}

void noop_code_listener::operator()(const add &instruction) {
  (void)instruction;
}

void noop_code_listener::operator()(const sub &instruction) {
  (void)instruction;
}

void noop_code_listener::operator()(const mul &instruction) {
  (void)instruction;
}

void noop_code_listener::operator()(const div &instruction) {
  (void)instruction;
}

void noop_code_listener::operator()(const mod &instruction) {
  (void)instruction;
}

void noop_code_listener::operator()(const lt &instruction) {
  (void)instruction;
}

void noop_code_listener::operator()(const le &instruction) {
  (void)instruction;
}

void noop_code_listener::operator()(const gt &instruction) {
  (void)instruction;
}

void noop_code_listener::operator()(const ge &instruction) {
  (void)instruction;
}

void noop_code_listener::operator()(const eq &instruction) {
  (void)instruction;
}

void noop_code_listener::operator()(const ne &instruction) {
  (void)instruction;
}

void noop_code_listener::operator()(const logical_and &instruction) {
  (void)instruction;
}

void noop_code_listener::operator()(const logical_or &instruction) {
  (void)instruction;
}

void noop_code_listener::operator()(const constant &instruction) {
  (void)instruction;
}

void noop_code_listener::operator()(const string &instruction) {
  (void)instruction;
}

void noop_code_listener::operator()(const sexp &instruction) {
  (void)instruction;
}

void noop_code_listener::operator()(const sti &instruction) {
  (void)instruction;
}

void noop_code_listener::operator()(const sta &instruction) {
  (void)instruction;
}

void noop_code_listener::operator()(const jmp &instruction) {
  (void)instruction;
}

void noop_code_listener::operator()(const end &instruction) {
  (void)instruction;
}

void noop_code_listener::operator()(const ret &instruction) {
  (void)instruction;
}

void noop_code_listener::operator()(const drop &instruction) {
  (void)instruction;
}

void noop_code_listener::operator()(const dup &instruction) {
  (void)instruction;
}

void noop_code_listener::operator()(const swap &instruction) {
  (void)instruction;
}

void noop_code_listener::operator()(const elem &instruction) {
  (void)instruction;
}

void noop_code_listener::operator()(const ld_glo &instruction) {
  (void)instruction;
}

void noop_code_listener::operator()(const ld_loc &instruction) {
  (void)instruction;
}

void noop_code_listener::operator()(const ld_arg &instruction) {
  (void)instruction;
}

void noop_code_listener::operator()(const ld_clo &instruction) {
  (void)instruction;
}

void noop_code_listener::operator()(const lda_glo &instruction) {
  (void)instruction;
}

void noop_code_listener::operator()(const lda_loc &instruction) {
  (void)instruction;
}

void noop_code_listener::operator()(const lda_arg &instruction) {
  (void)instruction;
}

void noop_code_listener::operator()(const lda_clo &instruction) {
  (void)instruction;
}

void noop_code_listener::operator()(const st_glo &instruction) {
  (void)instruction;
}

void noop_code_listener::operator()(const st_loc &instruction) {
  (void)instruction;
}

void noop_code_listener::operator()(const st_arg &instruction) {
  (void)instruction;
}

void noop_code_listener::operator()(const st_clo &instruction) {
  (void)instruction;
}

void noop_code_listener::operator()(const cjmp_z &instruction) {
  (void)instruction;
}

void noop_code_listener::operator()(const cjmp_nz &instruction) {
  (void)instruction;
}

void noop_code_listener::operator()(const begin &instruction) {
  (void)instruction;
}

void noop_code_listener::operator()(const begin_closure &instruction) {
  (void)instruction;
}

void noop_code_listener::operator()(
  const closure &instruction,
  std::span<const closure::capture> captures) {
  (void)instruction;
  (void)captures;
}

void noop_code_listener::operator()(const callc &instruction) {
  (void)instruction;
}

void noop_code_listener::operator()(const call &instruction) {
  (void)instruction;
}

void noop_code_listener::operator()(const tag &instruction) {
  (void)instruction;
}

void noop_code_listener::operator()(const array &instruction) {
  (void)instruction;
}

void noop_code_listener::operator()(const fail &instruction) {
  (void)instruction;
}

void noop_code_listener::operator()(const fail_keep &instruction) {
  (void)instruction;
}

void noop_code_listener::operator()(const line &instruction) {
  (void)instruction;
}

void noop_code_listener::operator()(const patt_str_cmp &instruction) {
  (void)instruction;
}

void noop_code_listener::operator()(const patt_string &instruction) {
  (void)instruction;
}

void noop_code_listener::operator()(const patt_array &instruction) {
  (void)instruction;
}

void noop_code_listener::operator()(const patt_sexp &instruction) {
  (void)instruction;
}

void noop_code_listener::operator()(const patt_boxed &instruction) {
  (void)instruction;
}

void noop_code_listener::operator()(const patt_unboxed &instruction) {
  (void)instruction;
}

void noop_code_listener::operator()(const patt_closure &instruction) {
  (void)instruction;
}

void noop_code_listener::operator()(const barray &instruction) {
  (void)instruction;
}

void noop_code_listener::operator()(const eof &instruction) {
  (void)instruction;
}

} // namespace lamavm::bc::test
