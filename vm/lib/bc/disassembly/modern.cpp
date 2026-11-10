#include "modern.hpp"

#include "bc/code_reader.hpp"
#include "bc/layout.hpp"
#include "core/exception.hpp"
#include "support/string.hpp"

#include <algorithm>
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
#include <variant>
#include <vector>

namespace lamavm::bc::disassembly {
namespace {

void print_field(std::ostream &out, std::string_view name, std::string_view value) {
  constexpr auto width = std::string_view("public_symbols_count:").size();
  out << std::format("  {:<{}}\t{}\n", name, width, value);
}

template <typename Instruction>
auto hex_bytes(const Instruction &instruction) -> std::string {
  const auto bytes = std::as_bytes(std::span(&instruction, 1));
  std::string result;
  result.reserve((bytes.size() * 3) - 1);
  for (const auto byte : bytes) {
    if (!result.empty()) {
      result += ' ';
    }

    result += std::format("{:02X}", std::to_integer<unsigned>(byte));
  }

  return result;
}

} // namespace

modern::modern(std::ostream &output, std::ostream &error_output)
  : out(&output)
  , err(&error_output) {
}

auto modern::string_at(std::int32_t offset) const -> std::string {
  if (offset < 0 || std::cmp_greater_equal(offset, strings.size())) {
    throw lamavm::exception(std::format("invalid string offset {}", offset));
  }

  const auto start = static_cast<std::size_t>(offset);
  const auto end = strings.find('\0', start);
  if (end == std::string::npos) {
    throw lamavm::exception(std::format("unterminated string at offset {}", offset));
  }

  return support::quote(std::string_view(strings).substr(start, end - start));
}

auto modern::target(std::int32_t value) const -> std::string {
  return value < 0 ? string_at(-(value + 1)) : std::to_string(value);
}

auto modern::capture_name(const closure::capture &capture) const -> std::string {
  switch (capture.kind) {
  case closure::capture::kind::global:
    return std::format("global {}", target(capture.index));
  case closure::capture::kind::local:
    return std::format("local {}", std::int32_t{capture.index});
  case closure::capture::kind::argument:
    return std::format("arg {}", std::int32_t{capture.index});
  case closure::capture::kind::captured:
    return std::format("captured {}", std::int32_t{capture.index});
  }

  throw lamavm::exception("invalid capture kind");
}

void modern::emit(std::size_t size, std::string bytes, std::string symbolic) {
  rows.push_back(
    {.offset = code_offset, .bytes = std::move(bytes), .symbolic = std::move(symbolic)});
  code_offset += size;
}

void modern::flush() {
  std::size_t address_width = 0;
  std::size_t bytes_width = 0;
  for (const auto &row : rows) {
    address_width = std::max(address_width, std::to_string(row.offset).size() + 1);
    bytes_width = std::max(bytes_width, row.bytes.size());
  }

  for (const auto &row : rows) {
    *out << std::format(
      "  {:<{}}\t{:<{}}\t{}\n",
      std::format("{}:", row.offset),
      address_width,
      row.bytes,
      bytes_width,
      row.symbolic);
  }

  rows.clear();
  code_offset = 0;
}

void modern::print_code_header() {
  if (!is_code_header_pending) {
    return;
  }

  if (symbol_index == 0) {
    *out << "\npublic_symbols:\n";
  }

  *out << "\ncode:\n";
  is_code_header_pending = false;
}

template <typename Instruction, typename... Args>
void modern::emit(
  const Instruction &instruction,
  std::format_string<Args...> symbolic,
  Args &&...args) {
  emit(
    sizeof(instruction),
    hex_bytes(instruction),
    std::format(symbolic, std::forward<Args>(args)...));
}

void modern::operator()(const header &instruction) {
  strings.clear();
  rows.clear();
  code_offset = 0;
  import_index = 0;
  symbol_index = 0;
  import_width = std::to_string(instruction.imports_count).size() + 1;
  symbol_width = std::to_string(instruction.public_symbols_count).size() + 1;
  is_code_header_pending = true;

  *out << "header:\n";
  print_field(
    *out,
    "magic:",
    support::quote(std::string_view(instruction.magic.data(), instruction.magic.size())));
  print_field(*out, "version:", std::to_string(instruction.version));
  print_field(*out, "string_table_size:", std::to_string(instruction.string_table_size));
  print_field(*out, "globals_count:", std::to_string(instruction.globals_count));
  print_field(*out, "imports_count:", std::to_string(instruction.imports_count));
  print_field(*out, "public_symbols_count:", std::to_string(instruction.public_symbols_count));
}

void modern::operator()(std::string_view table) {
  strings = table;
  *out << "\nstring_table:\n";
  const auto width = std::to_string(table.size()).size() + 1;
  for (std::size_t offset = 0; offset < table.size();) {
    *out << std::format(
      "  {:<{}}\t{}\n",
      std::format("{}:", offset),
      width,
      string_at(static_cast<std::int32_t>(offset)));
    offset = table.find('\0', offset) + 1;
  }

  *out << "\nimports:\n";
}

void modern::operator()(const import &entry) {
  *out << std::format(
    "  {:<{}}\t{} ({})\n",
    std::format("{}:", import_index),
    import_width,
    string_at(entry.name_offset),
    std::int32_t{entry.name_offset});
  ++import_index;
}

void modern::operator()(const public_symbol &entry) {
  if (symbol_index == 0) {
    *out << "\npublic_symbols:\n";
  }

  *out << std::format(
    "  {:<{}}\t{:<{}}\t{} ({}) at {}\n",
    std::format("{}:", symbol_index),
    symbol_width,
    entry.flag == public_symbol::flag::function ? "function" : "global",
    std::string_view("function").size(),
    string_at(entry.name_offset),
    std::int32_t{entry.name_offset},
    std::int32_t{entry.code_offset});
  ++symbol_index;
}

void modern::operator()(const add &instruction) {
  emit(instruction, "ADD");
}

void modern::operator()(const sub &instruction) {
  emit(instruction, "SUB");
}

void modern::operator()(const mul &instruction) {
  emit(instruction, "MUL");
}

void modern::operator()(const div &instruction) {
  emit(instruction, "DIV");
}

void modern::operator()(const mod &instruction) {
  emit(instruction, "MOD");
}

void modern::operator()(const lt &instruction) {
  emit(instruction, "LT");
}

void modern::operator()(const le &instruction) {
  emit(instruction, "LE");
}

void modern::operator()(const gt &instruction) {
  emit(instruction, "GT");
}

void modern::operator()(const ge &instruction) {
  emit(instruction, "GE");
}

void modern::operator()(const eq &instruction) {
  emit(instruction, "EQ");
}

void modern::operator()(const ne &instruction) {
  emit(instruction, "NE");
}

void modern::operator()(const logical_and &instruction) {
  emit(instruction, "LOGICAL_AND");
}

void modern::operator()(const logical_or &instruction) {
  emit(instruction, "LOGICAL_OR");
}

void modern::operator()(const sti &instruction) {
  emit(instruction, "STI");
}

void modern::operator()(const sta &instruction) {
  emit(instruction, "STA");
}

void modern::operator()(const end &instruction) {
  emit(instruction, "END");
}

void modern::operator()(const ret &instruction) {
  emit(instruction, "RET");
}

void modern::operator()(const drop &instruction) {
  emit(instruction, "DROP");
}

void modern::operator()(const dup &instruction) {
  emit(instruction, "DUP");
}

void modern::operator()(const swap &instruction) {
  emit(instruction, "SWAP");
}

void modern::operator()(const elem &instruction) {
  emit(instruction, "ELEM");
}

void modern::operator()(const patt_str_cmp &instruction) {
  emit(instruction, "PATT_STR_CMP");
}

void modern::operator()(const patt_string &instruction) {
  emit(instruction, "PATT_STRING");
}

void modern::operator()(const patt_array &instruction) {
  emit(instruction, "PATT_ARRAY");
}

void modern::operator()(const patt_sexp &instruction) {
  emit(instruction, "PATT_SEXP");
}

void modern::operator()(const patt_boxed &instruction) {
  emit(instruction, "PATT_BOXED");
}

void modern::operator()(const patt_unboxed &instruction) {
  emit(instruction, "PATT_UNBOXED");
}

void modern::operator()(const patt_closure &instruction) {
  emit(instruction, "PATT_CLOSURE");
}

void modern::operator()(const eof &instruction) {
  emit(instruction, "EOF");
}

void modern::operator()(const constant &instruction) {
  emit(instruction, "CONST {}", std::int32_t{instruction.value});
}

void modern::operator()(const array &instruction) {
  emit(instruction, "ARRAY {}", std::int32_t{instruction.n});
}

void modern::operator()(const barray &instruction) {
  emit(instruction, "BARRAY {}", std::int32_t{instruction.n});
}

void modern::operator()(const line &instruction) {
  emit(instruction, "LINE {}", std::int32_t{instruction.line});
}

void modern::operator()(const callc &instruction) {
  emit(instruction, "CALLC {}", std::int32_t{instruction.n_args});
}

void modern::operator()(const ld_glo &instruction) {
  emit(instruction, "LD_GLO {}", target(instruction.operand));
}

void modern::operator()(const ld_loc &instruction) {
  emit(instruction, "LD_LOC {}", std::int32_t{instruction.operand});
}

void modern::operator()(const ld_arg &instruction) {
  emit(instruction, "LD_ARG {}", std::int32_t{instruction.operand});
}

void modern::operator()(const ld_clo &instruction) {
  emit(instruction, "LD_CLO {}", std::int32_t{instruction.operand});
}

void modern::operator()(const lda_glo &instruction) {
  emit(instruction, "LDA_GLO {}", target(instruction.operand));
}

void modern::operator()(const lda_loc &instruction) {
  emit(instruction, "LDA_LOC {}", std::int32_t{instruction.operand});
}

void modern::operator()(const lda_arg &instruction) {
  emit(instruction, "LDA_ARG {}", std::int32_t{instruction.operand});
}

void modern::operator()(const lda_clo &instruction) {
  emit(instruction, "LDA_CLO {}", std::int32_t{instruction.operand});
}

void modern::operator()(const st_glo &instruction) {
  emit(instruction, "ST_GLO {}", target(instruction.operand));
}

void modern::operator()(const st_loc &instruction) {
  emit(instruction, "ST_LOC {}", std::int32_t{instruction.operand});
}

void modern::operator()(const st_arg &instruction) {
  emit(instruction, "ST_ARG {}", std::int32_t{instruction.operand});
}

void modern::operator()(const st_clo &instruction) {
  emit(instruction, "ST_CLO {}", std::int32_t{instruction.operand});
}

void modern::operator()(const jmp &instruction) {
  emit(instruction, "JMP {}", target(instruction.target));
}

void modern::operator()(const cjmp_z &instruction) {
  emit(instruction, "CJMP_Z {}", target(instruction.target));
}

void modern::operator()(const cjmp_nz &instruction) {
  emit(instruction, "CJMP_NZ {}", target(instruction.target));
}

void modern::operator()(const begin &instruction) {
  emit(
    instruction,
    "BEGIN {}, {}",
    std::int32_t{instruction.n_args},
    std::int32_t{instruction.n_locals});
}

void modern::operator()(const call &instruction) {
  emit(instruction, "CALL {}, {}", target(instruction.target), std::int32_t{instruction.n_args});
}

void modern::operator()(const fail &instruction) {
  emit(instruction, "FAIL {}, {}", std::int32_t{instruction.line}, std::int32_t{instruction.col});
}

void modern::operator()(const fail_keep &instruction) {
  emit(
    instruction,
    "FAIL_KEEP {}, {}",
    std::int32_t{instruction.line},
    std::int32_t{instruction.col});
}

void modern::operator()(const string &instruction) {
  emit(instruction, "STRING {}", string_at(instruction.string_offset));
}

void modern::operator()(const sexp &instruction) {
  emit(
    instruction,
    "SEXP {}, {}",
    string_at(instruction.tag_offset),
    std::int32_t{instruction.n_fields});
}

void modern::operator()(const tag &instruction) {
  emit(
    instruction,
    "TAG {}, {}",
    string_at(instruction.tag_offset),
    std::int32_t{instruction.n_fields});
}

void modern::operator()(const begin_closure &instruction) {
  emit(
    instruction,
    "BEGIN_CLOSURE {}, {}, {}",
    std::int32_t{instruction.n_args},
    std::int32_t{instruction.n_locals},
    std::int32_t{instruction.n_captured});
}

void modern::operator()(const closure &instruction, std::span<const closure::capture> captures) {
  auto bytes = hex_bytes(instruction);
  std::string captured;
  for (const auto &capture : captures) {
    bytes += ' ';
    bytes += hex_bytes(capture);
    if (!captured.empty()) {
      captured += ", ";
    }

    captured += capture_name(capture);
  }

  emit(
    sizeof(instruction) + captures.size_bytes(),
    std::move(bytes),
    std::format("CLOSURE {}, [{}]", target(instruction.target), captured));
}

void modern::operator()(std::size_t offset, std::string_view message) {
  *err << "offset " << offset << ": " << message << '\n';
}

auto modern::operator()(std::span<const std::byte> bytes) -> std::expected<void, std::monostate> {
  rows.clear();
  code_offset = 0;
  print_code_header();
  if (auto result = read_code(bytes, *this); !result) {
    rows.clear();
    code_offset = 0;
    return result;
  }

  flush();
  return {};
}

} // namespace lamavm::bc::disassembly
