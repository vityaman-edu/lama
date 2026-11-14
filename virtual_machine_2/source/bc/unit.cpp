#include "unit.hpp"

#include "layout.hpp"
#include "limit.hpp"

#include "core/assert.hpp"
#include "op/handler.hpp"

#include <array>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <format>
#include <fstream>
#include <ios>
#include <istream>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace lamavm::bc {

using namespace std::string_view_literals;

namespace {

class unit_parser {
public:
  explicit unit_parser(std::istream& in);
  unit read_unit();

private:
  template <typename T>
  T read();

  void read_bytes(char* data, std::size_t size);
  void read_header();
  void read_string_table();
  void read_imports();
  void read_public_symbols();
  void read_code_section();
  void fixup_jump_targets();

  std::istream& in; // NOLINT(cppcoreguidelines-avoid-const-or-ref-data-members)
  std::size_t cursor = 0;

  unit u;

  std::int32_t string_table_size = 0;
  std::int32_t imports_count = 0;
  std::int32_t public_symbols_count = 0;

  std::size_t metadata_size = 0;

  std::vector<std::int32_t> word_by_bc;
};

unit_parser::unit_parser(std::istream& in)
  : in(in) {
  static_assert(std::endian::native == std::endian::little);

  in.exceptions(std::ifstream::failbit | std::ifstream::badbit);
}

unit unit_parser::read_unit() {
  read_header();
  read_string_table();
  read_imports();
  read_public_symbols();
  read_code_section();
  fixup_jump_targets();
  return std::move(u);
}

void unit_parser::read_bytes(char* data, std::size_t size) {
  LAMAVM_EXPECT_B(size, <=, static_cast<std::size_t>(max_packed_unit_size) - cursor);
  in.read(data, static_cast<std::streamsize>(size));
  cursor += size;
}

template <typename T>
T unit_parser::read() {
  T value; // NOLINT(cppcoreguidelines-pro-type-member-init)
  read_bytes(reinterpret_cast<char*>(&value), sizeof(value));
  return value;
}

void unit_parser::read_header() {
  const auto magic = read<std::array<char, 4>>();
  const auto version = read<std::int32_t>();
  string_table_size = read<std::int32_t>();
  const auto globals_count = read<std::int32_t>();
  imports_count = read<std::int32_t>();
  public_symbols_count = read<std::int32_t>();

  const std::string_view header_magic(magic.data(), magic.size());
  LAMAVM_EXPECT_B(header_magic, ==, "LaMa"sv);
  LAMAVM_EXPECT_B(version, ==, 1);
  LAMAVM_EXPECT_B(0, <, string_table_size);
  LAMAVM_EXPECT_BB(0, <=, globals_count, <=, max_globals_count);
  LAMAVM_EXPECT_BB(0, <=, imports_count, <=, max_imports_count);
  LAMAVM_EXPECT_BB(0, <=, public_symbols_count, <=, max_public_symbols_count);

  metadata_size = cursor +
                  (string_table_size * sizeof(char)) +
                  (imports_count * sizeof(import_t)) +
                  (public_symbols_count * sizeof(public_symbol_t));
  LAMAVM_EXPECT_B(metadata_size, <, max_packed_unit_size);

  u.globals_count = globals_count;
}

void unit_parser::read_string_table() {
  u.string_table.resize(string_table_size);
  read_bytes(u.string_table.data(), u.string_table.size());
  LAMAVM_EXPECT_B(u.string_table.back(), ==, '\0');
}

void unit_parser::read_imports() {
  u.imports.reserve(imports_count);
  for (std::int32_t i = 0; i < imports_count; ++i) {
    const auto name_offset = read<std::int32_t>();

    LAMAVM_EXPECT_BB(0, <=, name_offset, <, string_table_size);

    u.imports.emplace_back(u.string_table.data() + name_offset);
  }
}

void unit_parser::read_public_symbols() {
  u.public_symbols.reserve(public_symbols_count);
  for (std::int32_t i = 0; i < public_symbols_count; ++i) {
    const auto name_offset = read<std::int32_t>();
    const auto code_offset = read<std::int32_t>();
    const auto flag = read<std::uint8_t>();

    LAMAVM_EXPECT_BB(0, <=, name_offset, <, string_table_size);
    LAMAVM_EXPECT_B(0, <=, code_offset);
    LAMAVM_EXPECT_B(flag, <=, 1U);

    u.public_symbols.emplace(
      u.string_table.data() + name_offset,
      public_symbol{
        .kind = static_cast<enum public_symbol::kind>(flag),
        .offset = code_offset,
      });
  }
}

void unit_parser::read_code_section() {
  for (;;) {
    const auto bc_offset = cursor - metadata_size;
    const auto opcode = read<opcode_t>();

    word_by_bc.resize(bc_offset + 1, -1);
    word_by_bc[bc_offset] = static_cast<std::int32_t>(u.text.size());

    u.text.push_back({.as_handler_id = static_cast<op::handler_id_t>(opcode)});

    if (opcode == opcode_t::op_eof) {
      LAMAVM_EXPECT_B(in.peek(), ==, std::char_traits<char>::eof());
      break;
    }

    std::size_t operands_count = 0;

    switch (opcode) {
    case opcode_t::op_add:
    case opcode_t::op_sub:
    case opcode_t::op_mul:
    case opcode_t::op_div:
    case opcode_t::op_mod:
    case opcode_t::op_lt:
    case opcode_t::op_le:
    case opcode_t::op_gt:
    case opcode_t::op_ge:
    case opcode_t::op_eq:
    case opcode_t::op_ne:
    case opcode_t::op_and:
    case opcode_t::op_or:
    case opcode_t::op_sta:
    case opcode_t::op_end:
    case opcode_t::op_drop:
    case opcode_t::op_dup:
    case opcode_t::op_swap:
    case opcode_t::op_elem:
    case opcode_t::op_patt_str_cmp:
    case opcode_t::op_patt_string:
    case opcode_t::op_patt_array:
    case opcode_t::op_patt_sexp:
    case opcode_t::op_patt_boxed:
    case opcode_t::op_patt_unboxed:
    case opcode_t::op_patt_closure:
      break;
    case opcode_t::op_const:
    case opcode_t::op_string:
    case opcode_t::op_jmp:
    case opcode_t::op_ld_glo:
    case opcode_t::op_ld_loc:
    case opcode_t::op_ld_arg:
    case opcode_t::op_ld_clo:
    case opcode_t::op_st_glo:
    case opcode_t::op_st_loc:
    case opcode_t::op_st_arg:
    case opcode_t::op_st_clo:
    case opcode_t::op_cjmp_z:
    case opcode_t::op_cjmp_nz:
    case opcode_t::op_callc:
    case opcode_t::op_array:
    case opcode_t::op_line:
    case opcode_t::op_barray:
      operands_count = 1;
      break;
    case opcode_t::op_sexp:
    case opcode_t::op_begin:
    case opcode_t::op_call:
    case opcode_t::op_tag:
    case opcode_t::op_fail:
    case opcode_t::op_fail_keep:
      operands_count = 2;
      break;
    case opcode_t::op_begin_closure:
      operands_count = 3;
      break;
    case opcode_t::op_closure: {
      const auto target = read<std::int32_t>();
      const auto n_captured = read<std::int32_t>();
      LAMAVM_EXPECT_B(0, <=, n_captured);
      u.text.push_back({.as_i32 = {.arg0 = target, .arg1 = n_captured}});

      for (std::int32_t i = 0; i < n_captured; ++i) {
        const auto kind = read<std::uint8_t>();
        const auto index = read<std::int32_t>();
        LAMAVM_EXPECT_B(kind, <=, 3U);
        u.text.push_back({.as_i32 = {.arg0 = kind, .arg1 = index}});
      }

      break;
    }
    case opcode_t::op_eof:
      std::unreachable();
    default:
      LAMAVM_EXPECT_C(
        false,
        std::format("unknown bytecode opcode 0x{:02x}", std::to_underlying(opcode)));
    }

    for (std::size_t i = 0; i < operands_count; i += 2) {
      const auto arg0 = read<std::int32_t>();
      const auto arg1 = i + 1 < operands_count ? read<std::int32_t>() : 0;

      u.text.push_back({.as_i32 = {.arg0 = arg0, .arg1 = arg1}});
    }
  }
}

void unit_parser::fixup_jump_targets() {
  const auto word_offset = [&](std::int32_t offset) {
    LAMAVM_EXPECT_B(0, <=, offset);
    LAMAVM_EXPECT_B(static_cast<std::size_t>(offset), <, word_by_bc.size());
    const auto word = word_by_bc[offset];
    LAMAVM_EXPECT_B(0, <=, word);
    return word;
  };

  for (std::size_t i = 0; i < u.text.size(); ++i) {
    const auto opcode = static_cast<opcode_t>(u.text[i].as_handler_id);

    switch (opcode) {
    case opcode_t::op_jmp:
    case opcode_t::op_cjmp_z:
    case opcode_t::op_cjmp_nz: {
      auto& target = u.text[++i].as_i32.arg0;
      target = word_offset(target);
      break;
    }
    case opcode_t::op_call:
    case opcode_t::op_closure: {
      auto& args = u.text[++i].as_i32;

      if (args.arg0 >= 0) {
        args.arg0 = word_offset(args.arg0);
      }

      if (opcode == opcode_t::op_closure) {
        i += args.arg1;
      }

      break;
    }
    case opcode_t::op_const:
    case opcode_t::op_string:
    case opcode_t::op_sexp:
    case opcode_t::op_ld_glo:
    case opcode_t::op_ld_loc:
    case opcode_t::op_ld_arg:
    case opcode_t::op_ld_clo:
    case opcode_t::op_st_glo:
    case opcode_t::op_st_loc:
    case opcode_t::op_st_arg:
    case opcode_t::op_st_clo:
    case opcode_t::op_begin:
    case opcode_t::op_callc:
    case opcode_t::op_tag:
    case opcode_t::op_array:
    case opcode_t::op_fail:
    case opcode_t::op_fail_keep:
    case opcode_t::op_line:
    case opcode_t::op_barray:
      ++i;
      break;
    case opcode_t::op_begin_closure:
      i += 2;
      break;
    default:
      break;
    }
  }

  for (auto& [name, symbol] : u.public_symbols) {
    if (symbol.kind == public_symbol::kind::function) {
      symbol.offset = word_offset(symbol.offset);
    }
  }
}

} // namespace

std::size_t unit::size(const unit& u) {
  return (u.string_table.size() * sizeof(decltype(u.string_table)::value_type)) +
         (u.text.size() * sizeof(decltype(u.text)::value_type)) +
         (u.imports.size() * sizeof(decltype(u.imports)::value_type)) +
         (u.public_symbols.size() * sizeof(decltype(u.public_symbols)::value_type));
}

unit unit::parse(std::istream& in) {
  return unit_parser(in).read_unit();
}

unit unit::parse(const std::filesystem::path& path) {
  LAMAVM_EXPECT_B(path.extension().string(), ==, ".bc"sv);

  std::ifstream f;
  f.exceptions(std::ifstream::failbit | std::ifstream::badbit);
  f.open(path, std::ifstream::binary);
  return parse(f);
}

} // namespace lamavm::bc
