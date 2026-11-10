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

void legacy::operator()(const header &op) {
  (void)op;

  strings.clear();
  code_offset = 0;
}

void legacy::operator()(std::string_view table) {
  strings = table;
}

void legacy::operator()(const import &op) {
  *out << std::format("IMPORT ({})\n", support::quote(string_at(op.name_offset)));
}

void legacy::operator()(const public_symbol &op) {
  *out << std::format(
    "PUBLIC ({}, {})\n",
    support::quote(string_at(op.name_offset)),
    op.flag == public_symbol::flag::function ? "true" : "false");
}

void legacy::operator()(const add &op) {
  emit(sizeof(op), "BINOP (\"+\")");
}

void legacy::operator()(const sub &op) {
  emit(sizeof(op), "BINOP (\"-\")");
}

void legacy::operator()(const mul &op) {
  emit(sizeof(op), "BINOP (\"*\")");
}

void legacy::operator()(const div &op) {
  emit(sizeof(op), "BINOP (\"/\")");
}

void legacy::operator()(const mod &op) {
  emit(sizeof(op), "BINOP (\"%\")");
}

void legacy::operator()(const lt &op) {
  emit(sizeof(op), "BINOP (\"<\")");
}

void legacy::operator()(const le &op) {
  emit(sizeof(op), "BINOP (\"<=\")");
}

void legacy::operator()(const gt &op) {
  emit(sizeof(op), "BINOP (\">\")");
}

void legacy::operator()(const ge &op) {
  emit(sizeof(op), "BINOP (\">=\")");
}

void legacy::operator()(const eq &op) {
  emit(sizeof(op), "BINOP (\"==\")");
}

void legacy::operator()(const ne &op) {
  emit(sizeof(op), "BINOP (\"!=\")");
}

void legacy::operator()(const logical_and &op) {
  emit(sizeof(op), "BINOP (\"&&\")");
}

void legacy::operator()(const logical_or &op) {
  emit(sizeof(op), "BINOP (\"!!\")");
}

void legacy::operator()(const sti &op) {
  emit(sizeof(op), "STI");
}

void legacy::operator()(const sta &op) {
  emit(sizeof(op), "STA");
}

void legacy::operator()(const end &op) {
  emit(sizeof(op), "END");
}

void legacy::operator()(const ret &op) {
  emit(sizeof(op), "RET");
}

void legacy::operator()(const drop &op) {
  emit(sizeof(op), "DROP");
}

void legacy::operator()(const dup &op) {
  emit(sizeof(op), "DUP");
}

void legacy::operator()(const swap &op) {
  emit(sizeof(op), "SWAP");
}

void legacy::operator()(const elem &op) {
  emit(sizeof(op), "ELEM");
}

void legacy::operator()(const constant &op) {
  emit(sizeof(op), std::format("CONST ({})", std::int32_t{op.value}));
}

void legacy::operator()(const array &op) {
  emit(sizeof(op), std::format("ARRAY ({})", std::int32_t{op.n}));
}

void legacy::operator()(const line &op) {
  emit(sizeof(op), std::format("LINE ({})", std::int32_t{op.line}));
}

void legacy::operator()(const string &op) {
  emit(sizeof(op), std::format("STRING ({})", support::quote(string_at(op.string_offset))));
}

void legacy::operator()(const sexp &op) {
  emit(
    sizeof(op),
    std::format(
      "SEXP ({}, {})",
      support::quote(string_at(op.tag_offset)),
      std::int32_t{op.n_fields}));
}

void legacy::operator()(const tag &op) {
  emit(
    sizeof(op),
    std::format(
      "TAG ({}, {})",
      support::quote(string_at(op.tag_offset)),
      std::int32_t{op.n_fields}));
}

void legacy::operator()(const jmp &op) {
  emit(sizeof(op), std::format("JMP ({})", target(op.target)));
}

void legacy::operator()(const cjmp_z &op) {
  emit(sizeof(op), std::format("CJMP (\"z\", {})", target(op.target)));
}

void legacy::operator()(const cjmp_nz &op) {
  emit(sizeof(op), std::format("CJMP (\"nz\", {})", target(op.target)));
}

void legacy::operator()(const ld_glo &op) {
  emit(sizeof(op), std::format("LD ({})", designation(closure::capture::kind::global, op.operand)));
}

void legacy::operator()(const ld_loc &op) {
  emit(sizeof(op), std::format("LD ({})", designation(closure::capture::kind::local, op.operand)));
}

void legacy::operator()(const ld_arg &op) {
  emit(
    sizeof(op),
    std::format("LD ({})", designation(closure::capture::kind::argument, op.operand)));
}

void legacy::operator()(const ld_clo &op) {
  emit(
    sizeof(op),
    std::format("LD ({})", designation(closure::capture::kind::captured, op.operand)));
}

void legacy::operator()(const lda_glo &op) {
  emit(
    sizeof(op),
    std::format("LDA ({})", designation(closure::capture::kind::global, op.operand)));
}

void legacy::operator()(const lda_loc &op) {
  emit(sizeof(op), std::format("LDA ({})", designation(closure::capture::kind::local, op.operand)));
}

void legacy::operator()(const lda_arg &op) {
  emit(
    sizeof(op),
    std::format("LDA ({})", designation(closure::capture::kind::argument, op.operand)));
}

void legacy::operator()(const lda_clo &op) {
  emit(
    sizeof(op),
    std::format("LDA ({})", designation(closure::capture::kind::captured, op.operand)));
}

void legacy::operator()(const st_glo &op) {
  emit(sizeof(op), std::format("ST ({})", designation(closure::capture::kind::global, op.operand)));
}

void legacy::operator()(const st_loc &op) {
  emit(sizeof(op), std::format("ST ({})", designation(closure::capture::kind::local, op.operand)));
}

void legacy::operator()(const st_arg &op) {
  emit(
    sizeof(op),
    std::format("ST ({})", designation(closure::capture::kind::argument, op.operand)));
}

void legacy::operator()(const st_clo &op) {
  emit(
    sizeof(op),
    std::format("ST ({})", designation(closure::capture::kind::captured, op.operand)));
}

void legacy::operator()(const begin &op) {
  const auto label = support::quote(std::format("0x{:08x}", code_offset));
  *out << std::format("LABEL ({})\n", label);
  emit(
    sizeof(op),
    std::format(
      "BEGIN ({}, {}, {}, [], [], [])",
      label,
      std::int32_t{op.n_args},
      std::int32_t{op.n_locals}));
}

void legacy::operator()(const begin_closure &op) {
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
}

void legacy::operator()(const closure &op, std::span<const closure::capture> captures) {
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
}

void legacy::operator()(const callc &op) {
  emit(sizeof(op), std::format("CALLC ({}, false)", std::int32_t{op.n_args}));
}

void legacy::operator()(const call &op) {
  emit(sizeof(op), std::format("CALL ({}, {}, false)", target(op.target), std::int32_t{op.n_args}));
}

void legacy::operator()(const barray &op) {
  emit(sizeof(op), std::format("CALL (\".array\", {}, false)", std::int32_t{op.n}));
}

void legacy::operator()(const fail &op) {
  emit(
    sizeof(op),
    std::format("FAIL (({}, {}), false)", std::int32_t{op.line}, std::int32_t{op.col}));
}

void legacy::operator()(const fail_keep &op) {
  emit(
    sizeof(op),
    std::format("FAIL (({}, {}), true)", std::int32_t{op.line}, std::int32_t{op.col}));
}

void legacy::operator()(const patt_str_cmp &op) {
  emit(sizeof(op), "PATT (StrCmp)");
}

void legacy::operator()(const patt_string &op) {
  emit(sizeof(op), "PATT (String)");
}

void legacy::operator()(const patt_array &op) {
  emit(sizeof(op), "PATT (Array)");
}

void legacy::operator()(const patt_sexp &op) {
  emit(sizeof(op), "PATT (Sexp)");
}

void legacy::operator()(const patt_boxed &op) {
  emit(sizeof(op), "PATT (Boxed)");
}

void legacy::operator()(const patt_unboxed &op) {
  emit(sizeof(op), "PATT (UnBoxed)");
}

void legacy::operator()(const patt_closure &op) {
  emit(sizeof(op), "PATT (Closure)");
}

void legacy::operator()(const eof &op) {
  (void)op;
}

auto legacy::operator()(std::span<const std::byte> bytes)
  -> std::expected<void, std::vector<std::string>> {
  return read_ops(bytes, *this);
}

} // namespace lamavm::bc::disassembly
