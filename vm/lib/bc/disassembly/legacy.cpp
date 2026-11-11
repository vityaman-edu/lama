#include "legacy.hpp"

#include "bc/code_reader.hpp"
#include "bc/layout.hpp"
#include "core/exception.hpp"
#include "support/string.hpp"

#include <cstddef>
#include <cstdint>
#include <expected>
#include <format>
#include <iosfwd>
#include <ostream>
#include <span>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace lamavm::bc::disassembly {

legacy::legacy(std::ostream &output)
  : out(&output) {
}

auto legacy::string_at(std::int32_t offset) const -> std::string_view {
  if (offset < 0 || std::cmp_greater_equal(offset, strings.size())) {
    throw lamavm::exception(std::format("invalid string offset {}", offset));
  }

  const auto start = static_cast<std::size_t>(offset);
  const auto end = strings.find('\0', start);
  if (end == std::string::npos) {
    throw lamavm::exception(std::format("unterminated string at offset {}", offset));
  }

  return std::string_view(strings).substr(start, end - start);
}

auto legacy::target(std::int32_t value) const -> std::string {
  if (value < 0) {
    return support::quote(string_at(-(value + 1)));
  }

  return support::quote(std::format("0x{:08x}", value));
}

auto legacy::designation(enum closure::capture::kind kind, std::int32_t index) const
  -> std::string {
  switch (kind) {
  case closure::capture::kind::global:
    return std::format(
      "Global ({})",
      index < 0 ? target(index) : support::quote(std::format("global_{}", index)));
  case closure::capture::kind::local:
    return std::format("Local ({})", index);
  case closure::capture::kind::argument:
    return std::format("Arg ({})", index);
  case closure::capture::kind::captured:
    return std::format("Access ({})", index);
  }

  throw lamavm::exception("invalid capture kind");
}

void legacy::emit(std::size_t size, std::string_view op) {
  *out << op << '\n';
  code_offset += size;
}

auto legacy::operator()(const header &op) -> status {
  (void)op;

  strings.clear();
  code_offset = 0;

  return {};
}

auto legacy::operator()(std::string_view table) -> status {
  strings = table;

  return {};
}

auto legacy::operator()(const import &op) -> status {
  *out << std::format("IMPORT ({})\n", support::quote(string_at(op.name_offset)));

  return {};
}

auto legacy::operator()(const public_symbol &op) -> status {
  *out << std::format(
    "PUBLIC ({}, {})\n",
    support::quote(string_at(op.name_offset)),
    op.flag == public_symbol::flag::function ? "true" : "false");

  return {};
}

auto legacy::operator()(const add &op) -> status {
  emit(sizeof(op), "BINOP (\"+\")");

  return {};
}

auto legacy::operator()(const sub &op) -> status {
  emit(sizeof(op), "BINOP (\"-\")");

  return {};
}

auto legacy::operator()(const mul &op) -> status {
  emit(sizeof(op), "BINOP (\"*\")");

  return {};
}

auto legacy::operator()(const div &op) -> status {
  emit(sizeof(op), "BINOP (\"/\")");

  return {};
}

auto legacy::operator()(const mod &op) -> status {
  emit(sizeof(op), "BINOP (\"%\")");

  return {};
}

auto legacy::operator()(const lt &op) -> status {
  emit(sizeof(op), "BINOP (\"<\")");

  return {};
}

auto legacy::operator()(const le &op) -> status {
  emit(sizeof(op), "BINOP (\"<=\")");

  return {};
}

auto legacy::operator()(const gt &op) -> status {
  emit(sizeof(op), "BINOP (\">\")");

  return {};
}

auto legacy::operator()(const ge &op) -> status {
  emit(sizeof(op), "BINOP (\">=\")");

  return {};
}

auto legacy::operator()(const eq &op) -> status {
  emit(sizeof(op), "BINOP (\"==\")");

  return {};
}

auto legacy::operator()(const ne &op) -> status {
  emit(sizeof(op), "BINOP (\"!=\")");

  return {};
}

auto legacy::operator()(const logical_and &op) -> status {
  emit(sizeof(op), "BINOP (\"&&\")");

  return {};
}

auto legacy::operator()(const logical_or &op) -> status {
  emit(sizeof(op), "BINOP (\"!!\")");

  return {};
}

auto legacy::operator()(const sti &op) -> status {
  emit(sizeof(op), "STI");

  return {};
}

auto legacy::operator()(const sta &op) -> status {
  emit(sizeof(op), "STA");

  return {};
}

auto legacy::operator()(const end &op) -> status {
  emit(sizeof(op), "END");

  return {};
}

auto legacy::operator()(const ret &op) -> status {
  emit(sizeof(op), "RET");

  return {};
}

auto legacy::operator()(const drop &op) -> status {
  emit(sizeof(op), "DROP");

  return {};
}

auto legacy::operator()(const dup &op) -> status {
  emit(sizeof(op), "DUP");

  return {};
}

auto legacy::operator()(const swap &op) -> status {
  emit(sizeof(op), "SWAP");

  return {};
}

auto legacy::operator()(const elem &op) -> status {
  emit(sizeof(op), "ELEM");

  return {};
}

auto legacy::operator()(const constant &op) -> status {
  emit(sizeof(op), std::format("CONST ({})", std::int32_t{op.value}));

  return {};
}

auto legacy::operator()(const array &op) -> status {
  emit(sizeof(op), std::format("ARRAY ({})", std::int32_t{op.n}));

  return {};
}

auto legacy::operator()(const line &op) -> status {
  emit(sizeof(op), std::format("LINE ({})", std::int32_t{op.line}));

  return {};
}

auto legacy::operator()(const string &op) -> status {
  emit(sizeof(op), std::format("STRING ({})", support::quote(string_at(op.string_offset))));

  return {};
}

auto legacy::operator()(const sexp &op) -> status {
  emit(
    sizeof(op),
    std::format(
      "SEXP ({}, {})",
      support::quote(string_at(op.tag_offset)),
      std::int32_t{op.n_fields}));

  return {};
}

auto legacy::operator()(const tag &op) -> status {
  emit(
    sizeof(op),
    std::format(
      "TAG ({}, {})",
      support::quote(string_at(op.tag_offset)),
      std::int32_t{op.n_fields}));

  return {};
}

auto legacy::operator()(const jmp &op) -> status {
  emit(sizeof(op), std::format("JMP ({})", target(op.target)));

  return {};
}

auto legacy::operator()(const cjmp_z &op) -> status {
  emit(sizeof(op), std::format("CJMP (\"z\", {})", target(op.target)));

  return {};
}

auto legacy::operator()(const cjmp_nz &op) -> status {
  emit(sizeof(op), std::format("CJMP (\"nz\", {})", target(op.target)));

  return {};
}

auto legacy::operator()(const ld_glo &op) -> status {
  emit(sizeof(op), std::format("LD ({})", designation(closure::capture::kind::global, op.operand)));

  return {};
}

auto legacy::operator()(const ld_loc &op) -> status {
  emit(sizeof(op), std::format("LD ({})", designation(closure::capture::kind::local, op.operand)));

  return {};
}

auto legacy::operator()(const ld_arg &op) -> status {
  emit(
    sizeof(op),
    std::format("LD ({})", designation(closure::capture::kind::argument, op.operand)));

  return {};
}

auto legacy::operator()(const ld_clo &op) -> status {
  emit(
    sizeof(op),
    std::format("LD ({})", designation(closure::capture::kind::captured, op.operand)));

  return {};
}

auto legacy::operator()(const lda_glo &op) -> status {
  emit(
    sizeof(op),
    std::format("LDA ({})", designation(closure::capture::kind::global, op.operand)));

  return {};
}

auto legacy::operator()(const lda_loc &op) -> status {
  emit(sizeof(op), std::format("LDA ({})", designation(closure::capture::kind::local, op.operand)));

  return {};
}

auto legacy::operator()(const lda_arg &op) -> status {
  emit(
    sizeof(op),
    std::format("LDA ({})", designation(closure::capture::kind::argument, op.operand)));

  return {};
}

auto legacy::operator()(const lda_clo &op) -> status {
  emit(
    sizeof(op),
    std::format("LDA ({})", designation(closure::capture::kind::captured, op.operand)));

  return {};
}

auto legacy::operator()(const st_glo &op) -> status {
  emit(sizeof(op), std::format("ST ({})", designation(closure::capture::kind::global, op.operand)));

  return {};
}

auto legacy::operator()(const st_loc &op) -> status {
  emit(sizeof(op), std::format("ST ({})", designation(closure::capture::kind::local, op.operand)));

  return {};
}

auto legacy::operator()(const st_arg &op) -> status {
  emit(
    sizeof(op),
    std::format("ST ({})", designation(closure::capture::kind::argument, op.operand)));

  return {};
}

auto legacy::operator()(const st_clo &op) -> status {
  emit(
    sizeof(op),
    std::format("ST ({})", designation(closure::capture::kind::captured, op.operand)));

  return {};
}

auto legacy::operator()(const begin &op) -> status {
  const auto label = support::quote(std::format("0x{:08x}", code_offset));
  *out << std::format("LABEL ({})\n", label);
  emit(
    sizeof(op),
    std::format(
      "BEGIN ({}, {}, {}, [], [], [])",
      label,
      std::int32_t{op.n_args},
      std::int32_t{op.n_locals}));

  return {};
}

auto legacy::operator()(const begin_closure &op) -> status {
  const auto label = support::quote(std::format("0x{:08x}", code_offset));
  std::string captured;
  for (std::int32_t i = 0; i < std::int32_t{op.n_captured}; ++i) {
    if (i != 0) {
      captured += "; ";
    }

    captured += designation(closure::capture::kind::captured, i);
  }

  *out << std::format("LABEL ({})\n", label);
  emit(
    sizeof(op),
    std::format(
      "BEGIN ({}, {}, {}, [{}], [], [])",
      label,
      std::int32_t{op.n_args},
      std::int32_t{op.n_locals},
      captured));

  return {};
}

auto legacy::operator()(const closure &op, std::span<const closure::capture> captures) -> status {
  std::string captured;
  for (const auto &capture : captures) {
    if (!captured.empty()) {
      captured += "; ";
    }

    captured += designation(capture.kind, capture.index);
  }

  emit(
    sizeof(op) + captures.size_bytes(),
    std::format("CLOSURE ({}, [{}])", target(op.target), captured));

  return {};
}

auto legacy::operator()(const callc &op) -> status {
  emit(sizeof(op), std::format("CALLC ({}, false)", std::int32_t{op.n_args}));

  return {};
}

auto legacy::operator()(const call &op) -> status {
  emit(sizeof(op), std::format("CALL ({}, {}, false)", target(op.target), std::int32_t{op.n_args}));

  return {};
}

auto legacy::operator()(const barray &op) -> status {
  emit(sizeof(op), std::format("CALL (\".array\", {}, false)", std::int32_t{op.n}));

  return {};
}

auto legacy::operator()(const fail &op) -> status {
  emit(
    sizeof(op),
    std::format("FAIL (({}, {}), false)", std::int32_t{op.line}, std::int32_t{op.col}));

  return {};
}

auto legacy::operator()(const fail_keep &op) -> status {
  emit(
    sizeof(op),
    std::format("FAIL (({}, {}), true)", std::int32_t{op.line}, std::int32_t{op.col}));

  return {};
}

auto legacy::operator()(const patt_str_cmp &op) -> status {
  emit(sizeof(op), "PATT (StrCmp)");

  return {};
}

auto legacy::operator()(const patt_string &op) -> status {
  emit(sizeof(op), "PATT (String)");

  return {};
}

auto legacy::operator()(const patt_array &op) -> status {
  emit(sizeof(op), "PATT (Array)");

  return {};
}

auto legacy::operator()(const patt_sexp &op) -> status {
  emit(sizeof(op), "PATT (Sexp)");

  return {};
}

auto legacy::operator()(const patt_boxed &op) -> status {
  emit(sizeof(op), "PATT (Boxed)");

  return {};
}

auto legacy::operator()(const patt_unboxed &op) -> status {
  emit(sizeof(op), "PATT (UnBoxed)");

  return {};
}

auto legacy::operator()(const patt_closure &op) -> status {
  emit(sizeof(op), "PATT (Closure)");

  return {};
}

auto legacy::operator()(const eof &op) -> status {
  (void)op;

  return {};
}

auto legacy::operator()(std::span<const std::byte> bytes)
  -> std::expected<void, std::vector<error>> {
  return read_ops(bytes, *this);
}

} // namespace lamavm::bc::disassembly
