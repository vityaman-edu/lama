#include "loop.hpp"

#include "bc/layout.hpp"
#include "core/exception.hpp"
#include "state.hpp"

#include <cstddef>
#include <expected>
#include <span>
#include <string>
#include <utility>
#include <vector>

namespace lamavm::interpreter {

namespace {

constexpr std::size_t stack_size = std::size_t{1024} * 1024;

} // namespace

loop::loop(std::string string_table)
  : state(std::move(string_table), std::vector<std::byte>(stack_size)) {
}

auto loop::operator()(const bc::add &op) -> status {
  (void)op;
  return std::unexpected("unsupported instruction: add");
}

auto loop::operator()(const bc::sub &op) -> status {
  (void)op;
  return std::unexpected("unsupported instruction: sub");
}

auto loop::operator()(const bc::mul &op) -> status {
  (void)op;
  return std::unexpected("unsupported instruction: mul");
}

auto loop::operator()(const bc::div &op) -> status {
  (void)op;
  return std::unexpected("unsupported instruction: div");
}

auto loop::operator()(const bc::mod &op) -> status {
  (void)op;
  return std::unexpected("unsupported instruction: mod");
}

auto loop::operator()(const bc::lt &op) -> status {
  (void)op;
  return std::unexpected("unsupported instruction: lt");
}

auto loop::operator()(const bc::le &op) -> status {
  (void)op;
  return std::unexpected("unsupported instruction: le");
}

auto loop::operator()(const bc::gt &op) -> status {
  (void)op;
  return std::unexpected("unsupported instruction: gt");
}

auto loop::operator()(const bc::ge &op) -> status {
  (void)op;
  return std::unexpected("unsupported instruction: ge");
}

auto loop::operator()(const bc::eq &op) -> status {
  (void)op;
  return std::unexpected("unsupported instruction: eq");
}

auto loop::operator()(const bc::ne &op) -> status {
  (void)op;
  return std::unexpected("unsupported instruction: ne");
}

auto loop::operator()(const bc::logical_and &op) -> status {
  (void)op;
  return std::unexpected("unsupported instruction: logical_and");
}

auto loop::operator()(const bc::logical_or &op) -> status {
  (void)op;
  return std::unexpected("unsupported instruction: logical_or");
}

auto loop::operator()(const bc::constant &op) -> status {
  (void)op;
  return std::unexpected("unsupported instruction: constant");
}

auto loop::operator()(const bc::string &op) -> status {
  (void)op;
  return std::unexpected("unsupported instruction: string");
}

auto loop::operator()(const bc::sexp &op) -> status {
  (void)op;
  return std::unexpected("unsupported instruction: sexp");
}

auto loop::operator()(const bc::sti &op) -> status {
  (void)op;
  return std::unexpected("unsupported instruction: sti");
}

auto loop::operator()(const bc::sta &op) -> status {
  (void)op;
  return std::unexpected("unsupported instruction: sta");
}

auto loop::operator()(const bc::jmp &op) -> status {
  (void)op;
  return std::unexpected("unsupported instruction: jmp");
}

auto loop::operator()(const bc::end &op) -> status {
  (void)op;
  return std::unexpected("unsupported instruction: end");
}

auto loop::operator()(const bc::ret &op) -> status {
  (void)op;
  return std::unexpected("unsupported instruction: ret");
}

auto loop::operator()(const bc::drop &op) -> status {
  (void)op;
  return std::unexpected("unsupported instruction: drop");
}

auto loop::operator()(const bc::dup &op) -> status {
  (void)op;
  return std::unexpected("unsupported instruction: dup");
}

auto loop::operator()(const bc::swap &op) -> status {
  (void)op;
  return std::unexpected("unsupported instruction: swap");
}

auto loop::operator()(const bc::elem &op) -> status {
  (void)op;
  return std::unexpected("unsupported instruction: elem");
}

auto loop::operator()(const bc::ld_glo &op) -> status {
  (void)op;
  return std::unexpected("unsupported instruction: ld_glo");
}

auto loop::operator()(const bc::ld_loc &op) -> status {
  (void)op;
  return std::unexpected("unsupported instruction: ld_loc");
}

auto loop::operator()(const bc::ld_arg &op) -> status {
  (void)op;
  return std::unexpected("unsupported instruction: ld_arg");
}

auto loop::operator()(const bc::ld_clo &op) -> status {
  (void)op;
  return std::unexpected("unsupported instruction: ld_clo");
}

auto loop::operator()(const bc::lda_glo &op) -> status {
  (void)op;
  return std::unexpected("unsupported instruction: lda_glo");
}

auto loop::operator()(const bc::lda_loc &op) -> status {
  (void)op;
  return std::unexpected("unsupported instruction: lda_loc");
}

auto loop::operator()(const bc::lda_arg &op) -> status {
  (void)op;
  return std::unexpected("unsupported instruction: lda_arg");
}

auto loop::operator()(const bc::lda_clo &op) -> status {
  (void)op;
  return std::unexpected("unsupported instruction: lda_clo");
}

auto loop::operator()(const bc::st_glo &op) -> status {
  (void)op;
  return std::unexpected("unsupported instruction: st_glo");
}

auto loop::operator()(const bc::st_loc &op) -> status {
  (void)op;
  return std::unexpected("unsupported instruction: st_loc");
}

auto loop::operator()(const bc::st_arg &op) -> status {
  (void)op;
  return std::unexpected("unsupported instruction: st_arg");
}

auto loop::operator()(const bc::st_clo &op) -> status {
  (void)op;
  return std::unexpected("unsupported instruction: st_clo");
}

auto loop::operator()(const bc::cjmp_z &op) -> status {
  (void)op;
  return std::unexpected("unsupported instruction: cjmp_z");
}

auto loop::operator()(const bc::cjmp_nz &op) -> status {
  (void)op;
  return std::unexpected("unsupported instruction: cjmp_nz");
}

auto loop::operator()(const bc::begin &op) -> status {
  (void)op;
  return std::unexpected("unsupported instruction: begin");
}

auto loop::operator()(const bc::begin_closure &op) -> status {
  (void)op;
  return std::unexpected("unsupported instruction: begin_closure");
}

auto loop::operator()(const bc::closure &op, std::span<const bc::closure::capture> captures)
  -> status {
  (void)op;
  (void)captures;
  return std::unexpected("unsupported instruction: closure");
}

auto loop::operator()(const bc::callc &op) -> status {
  (void)op;
  return std::unexpected("unsupported instruction: callc");
}

auto loop::operator()(const bc::call &op) -> status {
  (void)op;
  return std::unexpected("unsupported instruction: call");
}

auto loop::operator()(const bc::tag &op) -> status {
  (void)op;
  return std::unexpected("unsupported instruction: tag");
}

auto loop::operator()(const bc::array &op) -> status {
  (void)op;
  return std::unexpected("unsupported instruction: array");
}

auto loop::operator()(const bc::fail &op) -> status {
  (void)op;
  return std::unexpected("unsupported instruction: fail");
}

auto loop::operator()(const bc::fail_keep &op) -> status {
  (void)op;
  return std::unexpected("unsupported instruction: fail_keep");
}

auto loop::operator()(const bc::line &op) -> status {
  (void)op;
  return std::unexpected("unsupported instruction: line");
}

auto loop::operator()(const bc::patt_str_cmp &op) -> status {
  (void)op;
  return std::unexpected("unsupported instruction: patt_str_cmp");
}

auto loop::operator()(const bc::patt_string &op) -> status {
  (void)op;
  return std::unexpected("unsupported instruction: patt_string");
}

auto loop::operator()(const bc::patt_array &op) -> status {
  (void)op;
  return std::unexpected("unsupported instruction: patt_array");
}

auto loop::operator()(const bc::patt_sexp &op) -> status {
  (void)op;
  return std::unexpected("unsupported instruction: patt_sexp");
}

auto loop::operator()(const bc::patt_boxed &op) -> status {
  (void)op;
  return std::unexpected("unsupported instruction: patt_boxed");
}

auto loop::operator()(const bc::patt_unboxed &op) -> status {
  (void)op;
  return std::unexpected("unsupported instruction: patt_unboxed");
}

auto loop::operator()(const bc::patt_closure &op) -> status {
  (void)op;
  return std::unexpected("unsupported instruction: patt_closure");
}

auto loop::operator()(const bc::barray &op) -> status {
  (void)op;
  return std::unexpected("unsupported instruction: barray");
}

auto loop::operator()(const bc::eof &op) -> status {
  (void)op;
  return std::unexpected("unsupported instruction: eof");
}

} // namespace lamavm::interpreter
