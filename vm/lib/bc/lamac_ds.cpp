#include "lamac_ds.hpp"

#include "core/exception.hpp"
#include "layout.hpp"
#include "support/string.hpp"

#include <cstddef>
#include <cstdint>
#include <format>
#include <ostream>
#include <span>
#include <string>
#include <string_view>
#include <utility>

namespace lamavm::bc {

lamac_ds_listener::lamac_ds_listener(std::ostream &output)
  : out(&output) {
}

auto lamac_ds_listener::string_at(std::int32_t offset) const -> std::string_view {
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

auto lamac_ds_listener::target(std::int32_t value) const -> std::string {
  if (value < 0) {
    return support::quote(string_at(-(value + 1)));
  }

  return support::quote(std::format("0x{:08x}", value));
}

auto lamac_ds_listener::designation(enum closure::capture::kind kind, std::int32_t index) const
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

void lamac_ds_listener::emit(std::size_t size, std::string_view instruction) {
  *out << instruction << '\n';
  code_offset += size;
}

void lamac_ds_listener::operator()(const header &instruction) {
  (void)instruction;

  strings.clear();
  code_offset = 0;
}

void lamac_ds_listener::operator()(std::string_view table) {
  strings = table;
}

void lamac_ds_listener::operator()(const import &instruction) {
  *out << std::format("IMPORT ({})\n", support::quote(string_at(instruction.name_offset)));
}

void lamac_ds_listener::operator()(const public_symbol &instruction) {
  *out << std::format(
    "PUBLIC ({}, {})\n",
    support::quote(string_at(instruction.name_offset)),
    instruction.flag == public_symbol::flag::function ? "true" : "false");
}

void lamac_ds_listener::operator()(const add &instruction) {
  emit(sizeof(instruction), "BINOP (\"+\")");
}

void lamac_ds_listener::operator()(const sub &instruction) {
  emit(sizeof(instruction), "BINOP (\"-\")");
}

void lamac_ds_listener::operator()(const mul &instruction) {
  emit(sizeof(instruction), "BINOP (\"*\")");
}

void lamac_ds_listener::operator()(const div &instruction) {
  emit(sizeof(instruction), "BINOP (\"/\")");
}

void lamac_ds_listener::operator()(const mod &instruction) {
  emit(sizeof(instruction), "BINOP (\"%\")");
}

void lamac_ds_listener::operator()(const lt &instruction) {
  emit(sizeof(instruction), "BINOP (\"<\")");
}

void lamac_ds_listener::operator()(const le &instruction) {
  emit(sizeof(instruction), "BINOP (\"<=\")");
}

void lamac_ds_listener::operator()(const gt &instruction) {
  emit(sizeof(instruction), "BINOP (\">\")");
}

void lamac_ds_listener::operator()(const ge &instruction) {
  emit(sizeof(instruction), "BINOP (\">=\")");
}

void lamac_ds_listener::operator()(const eq &instruction) {
  emit(sizeof(instruction), "BINOP (\"==\")");
}

void lamac_ds_listener::operator()(const ne &instruction) {
  emit(sizeof(instruction), "BINOP (\"!=\")");
}

void lamac_ds_listener::operator()(const logical_and &instruction) {
  emit(sizeof(instruction), "BINOP (\"&&\")");
}

void lamac_ds_listener::operator()(const logical_or &instruction) {
  emit(sizeof(instruction), "BINOP (\"!!\")");
}

void lamac_ds_listener::operator()(const sti &instruction) {
  emit(sizeof(instruction), "STI");
}

void lamac_ds_listener::operator()(const sta &instruction) {
  emit(sizeof(instruction), "STA");
}

void lamac_ds_listener::operator()(const end &instruction) {
  emit(sizeof(instruction), "END");
}

void lamac_ds_listener::operator()(const ret &instruction) {
  emit(sizeof(instruction), "RET");
}

void lamac_ds_listener::operator()(const drop &instruction) {
  emit(sizeof(instruction), "DROP");
}

void lamac_ds_listener::operator()(const dup &instruction) {
  emit(sizeof(instruction), "DUP");
}

void lamac_ds_listener::operator()(const swap &instruction) {
  emit(sizeof(instruction), "SWAP");
}

void lamac_ds_listener::operator()(const elem &instruction) {
  emit(sizeof(instruction), "ELEM");
}

void lamac_ds_listener::operator()(const constant &instruction) {
  emit(sizeof(instruction), std::format("CONST ({})", std::int32_t{instruction.value}));
}

void lamac_ds_listener::operator()(const array &instruction) {
  emit(sizeof(instruction), std::format("ARRAY ({})", std::int32_t{instruction.n}));
}

void lamac_ds_listener::operator()(const line &instruction) {
  emit(sizeof(instruction), std::format("LINE ({})", std::int32_t{instruction.line}));
}

void lamac_ds_listener::operator()(const string &instruction) {
  emit(
    sizeof(instruction),
    std::format("STRING ({})", support::quote(string_at(instruction.string_offset))));
}

void lamac_ds_listener::operator()(const sexp &instruction) {
  emit(
    sizeof(instruction),
    std::format(
      "SEXP ({}, {})",
      support::quote(string_at(instruction.tag_offset)),
      std::int32_t{instruction.n_fields}));
}

void lamac_ds_listener::operator()(const tag &instruction) {
  emit(
    sizeof(instruction),
    std::format(
      "TAG ({}, {})",
      support::quote(string_at(instruction.tag_offset)),
      std::int32_t{instruction.n_fields}));
}

void lamac_ds_listener::operator()(const jmp &instruction) {
  emit(sizeof(instruction), std::format("JMP ({})", target(instruction.target)));
}

void lamac_ds_listener::operator()(const cjmp_z &instruction) {
  emit(sizeof(instruction), std::format("CJMP (\"z\", {})", target(instruction.target)));
}

void lamac_ds_listener::operator()(const cjmp_nz &instruction) {
  emit(sizeof(instruction), std::format("CJMP (\"nz\", {})", target(instruction.target)));
}

void lamac_ds_listener::operator()(const ld_glo &instruction) {
  emit(
    sizeof(instruction),
    std::format("LD ({})", designation(closure::capture::kind::global, instruction.operand)));
}

void lamac_ds_listener::operator()(const ld_loc &instruction) {
  emit(
    sizeof(instruction),
    std::format("LD ({})", designation(closure::capture::kind::local, instruction.operand)));
}

void lamac_ds_listener::operator()(const ld_arg &instruction) {
  emit(
    sizeof(instruction),
    std::format("LD ({})", designation(closure::capture::kind::argument, instruction.operand)));
}

void lamac_ds_listener::operator()(const ld_clo &instruction) {
  emit(
    sizeof(instruction),
    std::format("LD ({})", designation(closure::capture::kind::captured, instruction.operand)));
}

void lamac_ds_listener::operator()(const lda_glo &instruction) {
  emit(
    sizeof(instruction),
    std::format("LDA ({})", designation(closure::capture::kind::global, instruction.operand)));
}

void lamac_ds_listener::operator()(const lda_loc &instruction) {
  emit(
    sizeof(instruction),
    std::format("LDA ({})", designation(closure::capture::kind::local, instruction.operand)));
}

void lamac_ds_listener::operator()(const lda_arg &instruction) {
  emit(
    sizeof(instruction),
    std::format("LDA ({})", designation(closure::capture::kind::argument, instruction.operand)));
}

void lamac_ds_listener::operator()(const lda_clo &instruction) {
  emit(
    sizeof(instruction),
    std::format("LDA ({})", designation(closure::capture::kind::captured, instruction.operand)));
}

void lamac_ds_listener::operator()(const st_glo &instruction) {
  emit(
    sizeof(instruction),
    std::format("ST ({})", designation(closure::capture::kind::global, instruction.operand)));
}

void lamac_ds_listener::operator()(const st_loc &instruction) {
  emit(
    sizeof(instruction),
    std::format("ST ({})", designation(closure::capture::kind::local, instruction.operand)));
}

void lamac_ds_listener::operator()(const st_arg &instruction) {
  emit(
    sizeof(instruction),
    std::format("ST ({})", designation(closure::capture::kind::argument, instruction.operand)));
}

void lamac_ds_listener::operator()(const st_clo &instruction) {
  emit(
    sizeof(instruction),
    std::format("ST ({})", designation(closure::capture::kind::captured, instruction.operand)));
}

void lamac_ds_listener::operator()(const begin &instruction) {
  const auto label = support::quote(std::format("0x{:08x}", code_offset));
  *out << std::format("LABEL ({})\n", label);
  emit(
    sizeof(instruction),
    std::format(
      "BEGIN ({}, {}, {}, [], [], [])",
      label,
      std::int32_t{instruction.n_args},
      std::int32_t{instruction.n_locals}));
}

void lamac_ds_listener::operator()(const begin_closure &instruction) {
  const auto label = support::quote(std::format("0x{:08x}", code_offset));
  std::string captured;
  for (std::int32_t i = 0; i < std::int32_t{instruction.n_captured}; ++i) {
    if (i != 0) {
      captured += "; ";
    }

    captured += designation(closure::capture::kind::captured, i);
  }

  *out << std::format("LABEL ({})\n", label);
  emit(
    sizeof(instruction),
    std::format(
      "BEGIN ({}, {}, {}, [{}], [], [])",
      label,
      std::int32_t{instruction.n_args},
      std::int32_t{instruction.n_locals},
      captured));
}

void lamac_ds_listener::operator()(
  const closure &instruction,
  std::span<const closure::capture> captures) {
  std::string captured;
  for (const auto &capture : captures) {
    if (!captured.empty()) {
      captured += "; ";
    }

    captured += designation(capture.kind, capture.index);
  }

  emit(
    sizeof(instruction) + captures.size_bytes(),
    std::format("CLOSURE ({}, [{}])", target(instruction.target), captured));
}

void lamac_ds_listener::operator()(const callc &instruction) {
  emit(sizeof(instruction), std::format("CALLC ({}, false)", std::int32_t{instruction.n_args}));
}

void lamac_ds_listener::operator()(const call &instruction) {
  emit(
    sizeof(instruction),
    std::format(
      "CALL ({}, {}, false)",
      target(instruction.target),
      std::int32_t{instruction.n_args}));
}

void lamac_ds_listener::operator()(const barray &instruction) {
  emit(
    sizeof(instruction),
    std::format("CALL (\".array\", {}, false)", std::int32_t{instruction.n}));
}

void lamac_ds_listener::operator()(const fail &instruction) {
  emit(
    sizeof(instruction),
    std::format(
      "FAIL (({}, {}), false)",
      std::int32_t{instruction.line},
      std::int32_t{instruction.col}));
}

void lamac_ds_listener::operator()(const fail_keep &instruction) {
  emit(
    sizeof(instruction),
    std::format(
      "FAIL (({}, {}), true)",
      std::int32_t{instruction.line},
      std::int32_t{instruction.col}));
}

void lamac_ds_listener::operator()(const patt_str_cmp &instruction) {
  emit(sizeof(instruction), "PATT (StrCmp)");
}

void lamac_ds_listener::operator()(const patt_string &instruction) {
  emit(sizeof(instruction), "PATT (String)");
}

void lamac_ds_listener::operator()(const patt_array &instruction) {
  emit(sizeof(instruction), "PATT (Array)");
}

void lamac_ds_listener::operator()(const patt_sexp &instruction) {
  emit(sizeof(instruction), "PATT (Sexp)");
}

void lamac_ds_listener::operator()(const patt_boxed &instruction) {
  emit(sizeof(instruction), "PATT (Boxed)");
}

void lamac_ds_listener::operator()(const patt_unboxed &instruction) {
  emit(sizeof(instruction), "PATT (UnBoxed)");
}

void lamac_ds_listener::operator()(const patt_closure &instruction) {
  emit(sizeof(instruction), "PATT (Closure)");
}

void lamac_ds_listener::operator()(const eof &instruction) {
  (void)instruction;
}

}
