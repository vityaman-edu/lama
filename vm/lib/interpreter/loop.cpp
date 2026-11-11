#include "loop.hpp"

#include "bc/layout.hpp"
#include "core/exception.hpp"

#include <expected>
#include <span>

namespace lamavm::interpreter {

auto loop::operator()(const bc::add &op) -> status {
  (void)op;
  return std::unexpected("unsupported instruction");
}

auto loop::operator()(const bc::sub &op) -> status {
  (void)op;
  return std::unexpected("unsupported instruction");
}

auto loop::operator()(const bc::mul &op) -> status {
  (void)op;
  return std::unexpected("unsupported instruction");
}

auto loop::operator()(const bc::div &op) -> status {
  (void)op;
  return std::unexpected("unsupported instruction");
}

auto loop::operator()(const bc::mod &op) -> status {
  (void)op;
  return std::unexpected("unsupported instruction");
}

auto loop::operator()(const bc::lt &op) -> status {
  (void)op;
  return std::unexpected("unsupported instruction");
}

auto loop::operator()(const bc::le &op) -> status {
  (void)op;
  return std::unexpected("unsupported instruction");
}

auto loop::operator()(const bc::gt &op) -> status {
  (void)op;
  return std::unexpected("unsupported instruction");
}

auto loop::operator()(const bc::ge &op) -> status {
  (void)op;
  return std::unexpected("unsupported instruction");
}

auto loop::operator()(const bc::eq &op) -> status {
  (void)op;
  return std::unexpected("unsupported instruction");
}

auto loop::operator()(const bc::ne &op) -> status {
  (void)op;
  return std::unexpected("unsupported instruction");
}

auto loop::operator()(const bc::logical_and &op) -> status {
  (void)op;
  return std::unexpected("unsupported instruction");
}

auto loop::operator()(const bc::logical_or &op) -> status {
  (void)op;
  return std::unexpected("unsupported instruction");
}

auto loop::operator()(const bc::constant &op) -> status {
  (void)op;
  return std::unexpected("unsupported instruction");
}

auto loop::operator()(const bc::string &op) -> status {
  (void)op;
  return std::unexpected("unsupported instruction");
}

auto loop::operator()(const bc::sexp &op) -> status {
  (void)op;
  return std::unexpected("unsupported instruction");
}

auto loop::operator()(const bc::sti &op) -> status {
  (void)op;
  return std::unexpected("unsupported instruction");
}

auto loop::operator()(const bc::sta &op) -> status {
  (void)op;
  return std::unexpected("unsupported instruction");
}

auto loop::operator()(const bc::jmp &op) -> status {
  (void)op;
  return std::unexpected("unsupported instruction");
}

auto loop::operator()(const bc::end &op) -> status {
  (void)op;
  return std::unexpected("unsupported instruction");
}

auto loop::operator()(const bc::ret &op) -> status {
  (void)op;
  return std::unexpected("unsupported instruction");
}

auto loop::operator()(const bc::drop &op) -> status {
  (void)op;
  return std::unexpected("unsupported instruction");
}

auto loop::operator()(const bc::dup &op) -> status {
  (void)op;
  return std::unexpected("unsupported instruction");
}

auto loop::operator()(const bc::swap &op) -> status {
  (void)op;
  return std::unexpected("unsupported instruction");
}

auto loop::operator()(const bc::elem &op) -> status {
  (void)op;
  return std::unexpected("unsupported instruction");
}

auto loop::operator()(const bc::ld_glo &op) -> status {
  (void)op;
  return std::unexpected("unsupported instruction");
}

auto loop::operator()(const bc::ld_loc &op) -> status {
  (void)op;
  return std::unexpected("unsupported instruction");
}

auto loop::operator()(const bc::ld_arg &op) -> status {
  (void)op;
  return std::unexpected("unsupported instruction");
}

auto loop::operator()(const bc::ld_clo &op) -> status {
  (void)op;
  return std::unexpected("unsupported instruction");
}

auto loop::operator()(const bc::lda_glo &op) -> status {
  (void)op;
  return std::unexpected("unsupported instruction");
}

auto loop::operator()(const bc::lda_loc &op) -> status {
  (void)op;
  return std::unexpected("unsupported instruction");
}

auto loop::operator()(const bc::lda_arg &op) -> status {
  (void)op;
  return std::unexpected("unsupported instruction");
}

auto loop::operator()(const bc::lda_clo &op) -> status {
  (void)op;
  return std::unexpected("unsupported instruction");
}

auto loop::operator()(const bc::st_glo &op) -> status {
  (void)op;
  return std::unexpected("unsupported instruction");
}

auto loop::operator()(const bc::st_loc &op) -> status {
  (void)op;
  return std::unexpected("unsupported instruction");
}

auto loop::operator()(const bc::st_arg &op) -> status {
  (void)op;
  return std::unexpected("unsupported instruction");
}

auto loop::operator()(const bc::st_clo &op) -> status {
  (void)op;
  return std::unexpected("unsupported instruction");
}

auto loop::operator()(const bc::cjmp_z &op) -> status {
  (void)op;
  return std::unexpected("unsupported instruction");
}

auto loop::operator()(const bc::cjmp_nz &op) -> status {
  (void)op;
  return std::unexpected("unsupported instruction");
}

auto loop::operator()(const bc::begin &op) -> status {
  (void)op;
  return std::unexpected("unsupported instruction");
}

auto loop::operator()(const bc::begin_closure &op) -> status {
  (void)op;
  return std::unexpected("unsupported instruction");
}

auto loop::operator()(const bc::closure &op, std::span<const bc::closure::capture> captures)
  -> status {
  (void)op;
  (void)captures;
  return std::unexpected("unsupported instruction");
}

auto loop::operator()(const bc::callc &op) -> status {
  (void)op;
  return std::unexpected("unsupported instruction");
}

auto loop::operator()(const bc::call &op) -> status {
  (void)op;
  return std::unexpected("unsupported instruction");
}

auto loop::operator()(const bc::tag &op) -> status {
  (void)op;
  return std::unexpected("unsupported instruction");
}

auto loop::operator()(const bc::array &op) -> status {
  (void)op;
  return std::unexpected("unsupported instruction");
}

auto loop::operator()(const bc::fail &op) -> status {
  (void)op;
  return std::unexpected("unsupported instruction");
}

auto loop::operator()(const bc::fail_keep &op) -> status {
  (void)op;
  return std::unexpected("unsupported instruction");
}

auto loop::operator()(const bc::line &op) -> status {
  (void)op;
  return std::unexpected("unsupported instruction");
}

auto loop::operator()(const bc::patt_str_cmp &op) -> status {
  (void)op;
  return std::unexpected("unsupported instruction");
}

auto loop::operator()(const bc::patt_string &op) -> status {
  (void)op;
  return std::unexpected("unsupported instruction");
}

auto loop::operator()(const bc::patt_array &op) -> status {
  (void)op;
  return std::unexpected("unsupported instruction");
}

auto loop::operator()(const bc::patt_sexp &op) -> status {
  (void)op;
  return std::unexpected("unsupported instruction");
}

auto loop::operator()(const bc::patt_boxed &op) -> status {
  (void)op;
  return std::unexpected("unsupported instruction");
}

auto loop::operator()(const bc::patt_unboxed &op) -> status {
  (void)op;
  return std::unexpected("unsupported instruction");
}

auto loop::operator()(const bc::patt_closure &op) -> status {
  (void)op;
  return std::unexpected("unsupported instruction");
}

auto loop::operator()(const bc::barray &op) -> status {
  (void)op;
  return std::unexpected("unsupported instruction");
}

auto loop::operator()(const bc::eof &op) -> status {
  (void)op;
  return std::unexpected("unsupported instruction");
}

} // namespace lamavm::interpreter
