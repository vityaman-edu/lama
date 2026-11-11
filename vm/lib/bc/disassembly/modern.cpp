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
#include <vector>

namespace lamavm::bc::disassembly {
namespace {

void print_field(std::ostream &out, std::string_view name, std::string_view value) {
  constexpr auto width = std::string_view("public_symbols_count:").size();
  out << std::format("  {:<{}}\t{}\n", name, width, value);
}

template <typename Op>
auto hex_bytes(const Op &op) -> std::string {
  const auto bytes = std::as_bytes(std::span(&op, 1));
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

modern::modern(std::ostream &output)
  : out(&output) {
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

template <typename Op, typename... Args>
void modern::emit(const Op &op, std::format_string<Args...> symbolic, Args &&...args) {
  emit(sizeof(op), hex_bytes(op), std::format(symbolic, std::forward<Args>(args)...));
}

auto modern::operator()(const header &op) -> status {
  strings.clear();
  rows.clear();
  code_offset = 0;
  import_index = 0;
  symbol_index = 0;
  import_width = std::to_string(op.imports_count).size() + 1;
  symbol_width = std::to_string(op.public_symbols_count).size() + 1;
  is_code_header_pending = true;

  *out << "header:\n";
  print_field(*out, "magic:", support::quote(std::string_view(op.magic.data(), op.magic.size())));
  print_field(*out, "version:", std::to_string(op.version));
  print_field(*out, "string_table_size:", std::to_string(op.string_table_size));
  print_field(*out, "globals_count:", std::to_string(op.globals_count));
  print_field(*out, "imports_count:", std::to_string(op.imports_count));
  print_field(*out, "public_symbols_count:", std::to_string(op.public_symbols_count));

  return {};
}

auto modern::operator()(std::string &table) -> status {
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

  return {};
}

auto modern::operator()(const import &entry) -> status {
  *out << std::format(
    "  {:<{}}\t{} ({})\n",
    std::format("{}:", import_index),
    import_width,
    string_at(entry.name_offset),
    std::int32_t{entry.name_offset});
  ++import_index;

  return {};
}

auto modern::operator()(const public_symbol &entry) -> status {
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

  return {};
}

auto modern::operator()(const add &op) -> status {
  emit(op, "ADD");

  return {};
}

auto modern::operator()(const sub &op) -> status {
  emit(op, "SUB");

  return {};
}

auto modern::operator()(const mul &op) -> status {
  emit(op, "MUL");

  return {};
}

auto modern::operator()(const div &op) -> status {
  emit(op, "DIV");

  return {};
}

auto modern::operator()(const mod &op) -> status {
  emit(op, "MOD");

  return {};
}

auto modern::operator()(const lt &op) -> status {
  emit(op, "LT");

  return {};
}

auto modern::operator()(const le &op) -> status {
  emit(op, "LE");

  return {};
}

auto modern::operator()(const gt &op) -> status {
  emit(op, "GT");

  return {};
}

auto modern::operator()(const ge &op) -> status {
  emit(op, "GE");

  return {};
}

auto modern::operator()(const eq &op) -> status {
  emit(op, "EQ");

  return {};
}

auto modern::operator()(const ne &op) -> status {
  emit(op, "NE");

  return {};
}

auto modern::operator()(const logical_and &op) -> status {
  emit(op, "LOGICAL_AND");

  return {};
}

auto modern::operator()(const logical_or &op) -> status {
  emit(op, "LOGICAL_OR");

  return {};
}

auto modern::operator()(const sti &op) -> status {
  emit(op, "STI");

  return {};
}

auto modern::operator()(const sta &op) -> status {
  emit(op, "STA");

  return {};
}

auto modern::operator()(const end &op) -> status {
  emit(op, "END");

  return {};
}

auto modern::operator()(const ret &op) -> status {
  emit(op, "RET");

  return {};
}

auto modern::operator()(const drop &op) -> status {
  emit(op, "DROP");

  return {};
}

auto modern::operator()(const dup &op) -> status {
  emit(op, "DUP");

  return {};
}

auto modern::operator()(const swap &op) -> status {
  emit(op, "SWAP");

  return {};
}

auto modern::operator()(const elem &op) -> status {
  emit(op, "ELEM");

  return {};
}

auto modern::operator()(const patt_str_cmp &op) -> status {
  emit(op, "PATT_STR_CMP");

  return {};
}

auto modern::operator()(const patt_string &op) -> status {
  emit(op, "PATT_STRING");

  return {};
}

auto modern::operator()(const patt_array &op) -> status {
  emit(op, "PATT_ARRAY");

  return {};
}

auto modern::operator()(const patt_sexp &op) -> status {
  emit(op, "PATT_SEXP");

  return {};
}

auto modern::operator()(const patt_boxed &op) -> status {
  emit(op, "PATT_BOXED");

  return {};
}

auto modern::operator()(const patt_unboxed &op) -> status {
  emit(op, "PATT_UNBOXED");

  return {};
}

auto modern::operator()(const patt_closure &op) -> status {
  emit(op, "PATT_CLOSURE");

  return {};
}

auto modern::operator()(const eof &op) -> status {
  emit(op, "EOF");

  return {};
}

auto modern::operator()(const constant &op) -> status {
  emit(op, "CONST {}", std::int32_t{op.value});

  return {};
}

auto modern::operator()(const array &op) -> status {
  emit(op, "ARRAY {}", std::int32_t{op.n});

  return {};
}

auto modern::operator()(const barray &op) -> status {
  emit(op, "BARRAY {}", std::int32_t{op.n});

  return {};
}

auto modern::operator()(const line &op) -> status {
  emit(op, "LINE {}", std::int32_t{op.line});

  return {};
}

auto modern::operator()(const callc &op) -> status {
  emit(op, "CALLC {}", std::int32_t{op.n_args});

  return {};
}

auto modern::operator()(const ld_glo &op) -> status {
  emit(op, "LD_GLO {}", target(op.operand));

  return {};
}

auto modern::operator()(const ld_loc &op) -> status {
  emit(op, "LD_LOC {}", std::int32_t{op.operand});

  return {};
}

auto modern::operator()(const ld_arg &op) -> status {
  emit(op, "LD_ARG {}", std::int32_t{op.operand});

  return {};
}

auto modern::operator()(const ld_clo &op) -> status {
  emit(op, "LD_CLO {}", std::int32_t{op.operand});

  return {};
}

auto modern::operator()(const lda_glo &op) -> status {
  emit(op, "LDA_GLO {}", target(op.operand));

  return {};
}

auto modern::operator()(const lda_loc &op) -> status {
  emit(op, "LDA_LOC {}", std::int32_t{op.operand});

  return {};
}

auto modern::operator()(const lda_arg &op) -> status {
  emit(op, "LDA_ARG {}", std::int32_t{op.operand});

  return {};
}

auto modern::operator()(const lda_clo &op) -> status {
  emit(op, "LDA_CLO {}", std::int32_t{op.operand});

  return {};
}

auto modern::operator()(const st_glo &op) -> status {
  emit(op, "ST_GLO {}", target(op.operand));

  return {};
}

auto modern::operator()(const st_loc &op) -> status {
  emit(op, "ST_LOC {}", std::int32_t{op.operand});

  return {};
}

auto modern::operator()(const st_arg &op) -> status {
  emit(op, "ST_ARG {}", std::int32_t{op.operand});

  return {};
}

auto modern::operator()(const st_clo &op) -> status {
  emit(op, "ST_CLO {}", std::int32_t{op.operand});

  return {};
}

auto modern::operator()(const jmp &op) -> status {
  emit(op, "JMP {}", target(op.target));

  return {};
}

auto modern::operator()(const cjmp_z &op) -> status {
  emit(op, "CJMP_Z {}", target(op.target));

  return {};
}

auto modern::operator()(const cjmp_nz &op) -> status {
  emit(op, "CJMP_NZ {}", target(op.target));

  return {};
}

auto modern::operator()(const begin &op) -> status {
  emit(op, "BEGIN {}, {}", std::int32_t{op.n_args}, std::int32_t{op.n_locals});

  return {};
}

auto modern::operator()(const call &op) -> status {
  emit(op, "CALL {}, {}", target(op.target), std::int32_t{op.n_args});

  return {};
}

auto modern::operator()(const fail &op) -> status {
  emit(op, "FAIL {}, {}", std::int32_t{op.line}, std::int32_t{op.col});

  return {};
}

auto modern::operator()(const fail_keep &op) -> status {
  emit(op, "FAIL_KEEP {}, {}", std::int32_t{op.line}, std::int32_t{op.col});

  return {};
}

auto modern::operator()(const string &op) -> status {
  emit(op, "STRING {}", string_at(op.string_offset));

  return {};
}

auto modern::operator()(const sexp &op) -> status {
  emit(op, "SEXP {}, {}", string_at(op.tag_offset), std::int32_t{op.n_fields});

  return {};
}

auto modern::operator()(const tag &op) -> status {
  emit(op, "TAG {}, {}", string_at(op.tag_offset), std::int32_t{op.n_fields});

  return {};
}

auto modern::operator()(const begin_closure &op) -> status {
  emit(
    op,
    "BEGIN_CLOSURE {}, {}, {}",
    std::int32_t{op.n_args},
    std::int32_t{op.n_locals},
    std::int32_t{op.n_captured});

  return {};
}

auto modern::operator()(const closure &op, std::span<const closure::capture> captures) -> status {
  auto bytes = hex_bytes(op);
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
    sizeof(op) + captures.size_bytes(),
    std::move(bytes),
    std::format("CLOSURE {}, [{}]", target(op.target), captured));

  return {};
}

auto modern::operator()(std::span<const std::byte> bytes)
  -> std::expected<void, std::vector<error>> {
  rows.clear();
  code_offset = 0;
  print_code_header();
  if (auto result = read_ops(bytes, *this); !result) {
    rows.clear();
    code_offset = 0;
    return result;
  }

  flush();
  return {};
}

} // namespace lamavm::bc::disassembly
