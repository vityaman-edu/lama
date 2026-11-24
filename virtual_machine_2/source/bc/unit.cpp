#include "unit.hpp"

#include "layout.hpp"
#include "limit.hpp"

#include "core/assert.hpp"
#include "core/format.hpp"

#include "interpreter/cold/layout.hpp"

#include "op/word.hpp"

#include <array>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <cstring>
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

namespace cold = interpreter::cold;

class unit_parser {
public:
  explicit unit_parser(std::istream& in);
  unit read_unit();

private:
  template <typename T>
  T read();

  template <typename Packed>
  Packed read_op();

  template <typename Cold>
  void emit_op(const Cold& cold);

  closure_capture_t read_capture();
  void emit_capture(const cold::closure_capture_t& capture);

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
  T value; // NOLINT(cppcoreguidelines-pro-type-member-init, hicpp-member-init)
  read_bytes(reinterpret_cast<char*>(&value), sizeof(value));
  return value;
}

template <typename Packed>
Packed unit_parser::read_op() {
  static_assert(offsetof(Packed, opcode) == 0);
  return read<Packed>();
}

template <typename Cold>
void unit_parser::emit_op(const Cold& cold) {
  static_assert(offsetof(Cold, as_handler) == 0);

  const auto offset = u.code_section.size();
  u.code_section.resize(offset + op::word_count<Cold>());
  std::memcpy(u.code_section.data() + offset, &cold, sizeof(cold));
}

closure_capture_t unit_parser::read_capture() {
  const auto capture = read<closure_capture_t>();
  const auto kind = std::to_underlying(capture.kind);
  LAMAVM_EXPECT_B(kind, <=, std::to_underlying(capture_kind::closure));
  return capture;
}

void unit_parser::emit_capture(const cold::closure_capture_t& capture) {
  u.code_section.push_back({.as_i32 = std::bit_cast<decltype(op::word_t::as_i32)>(capture)});
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
    const auto opcode = static_cast<opcode_t>(in.peek());

    word_by_bc.resize(bc_offset + 1, -1);
    word_by_bc[bc_offset] = static_cast<std::int32_t>(u.code_section.size());

    switch (opcode) {
    case opcode_t::op_add: {
      const auto packed = read_op<op_add_t>();
      const cold::op_add_t op = {
        .as_handler = {.as_id = std::to_underlying(packed.opcode)},
      };
      emit_op(op);
      continue;
    }

    case opcode_t::op_sub: {
      const auto packed = read_op<op_sub_t>();
      const cold::op_sub_t op = {
        .as_handler = {.as_id = std::to_underlying(packed.opcode)},
      };
      emit_op(op);
      continue;
    }

    case opcode_t::op_mul: {
      const auto packed = read_op<op_mul_t>();
      const cold::op_mul_t op = {
        .as_handler = {.as_id = std::to_underlying(packed.opcode)},
      };
      emit_op(op);
      continue;
    }

    case opcode_t::op_div: {
      const auto packed = read_op<op_div_t>();
      const cold::op_div_t op = {
        .as_handler = {.as_id = std::to_underlying(packed.opcode)},
      };
      emit_op(op);
      continue;
    }

    case opcode_t::op_mod: {
      const auto packed = read_op<op_mod_t>();
      const cold::op_mod_t op = {
        .as_handler = {.as_id = std::to_underlying(packed.opcode)},
      };
      emit_op(op);
      continue;
    }

    case opcode_t::op_lt: {
      const auto packed = read_op<op_lt_t>();
      const cold::op_lt_t op = {
        .as_handler = {.as_id = std::to_underlying(packed.opcode)},
      };
      emit_op(op);
      continue;
    }

    case opcode_t::op_le: {
      const auto packed = read_op<op_le_t>();
      const cold::op_le_t op = {
        .as_handler = {.as_id = std::to_underlying(packed.opcode)},
      };
      emit_op(op);
      continue;
    }

    case opcode_t::op_gt: {
      const auto packed = read_op<op_gt_t>();
      const cold::op_gt_t op = {
        .as_handler = {.as_id = std::to_underlying(packed.opcode)},
      };
      emit_op(op);
      continue;
    }

    case opcode_t::op_ge: {
      const auto packed = read_op<op_ge_t>();
      const cold::op_ge_t op = {
        .as_handler = {.as_id = std::to_underlying(packed.opcode)},
      };
      emit_op(op);
      continue;
    }

    case opcode_t::op_eq: {
      const auto packed = read_op<op_eq_t>();
      const cold::op_eq_t op = {
        .as_handler = {.as_id = std::to_underlying(packed.opcode)},
      };
      emit_op(op);
      continue;
    }

    case opcode_t::op_ne: {
      const auto packed = read_op<op_ne_t>();
      const cold::op_ne_t op = {
        .as_handler = {.as_id = std::to_underlying(packed.opcode)},
      };
      emit_op(op);
      continue;
    }

    case opcode_t::op_and: {
      const auto packed = read_op<op_and_t>();
      const cold::op_and_t op = {
        .as_handler = {.as_id = std::to_underlying(packed.opcode)},
      };
      emit_op(op);
      continue;
    }

    case opcode_t::op_or: {
      const auto packed = read_op<op_or_t>();
      const cold::op_or_t op = {
        .as_handler = {.as_id = std::to_underlying(packed.opcode)},
      };
      emit_op(op);
      continue;
    }

    case opcode_t::op_const: {
      const auto packed = read_op<op_const_t>();
      const cold::op_const_t op = {
        .as_handler = {.as_id = std::to_underlying(packed.opcode)},
        .value = packed.value,
        .reserved_1 = 0,
      };
      emit_op(op);
      continue;
    }

    case opcode_t::op_string: {
      const auto packed = read_op<op_string_t>();
      const cold::op_string_t op = {
        .as_handler = {.as_id = std::to_underlying(packed.opcode)},
        .string_offset = packed.string_offset,
        .reserved_1 = 0,
      };
      emit_op(op);
      continue;
    }

    case opcode_t::op_sexp: {
      const auto packed = read_op<op_sexp_t>();
      const cold::op_sexp_t op = {
        .as_handler = {.as_id = std::to_underlying(packed.opcode)},
        .tag_offset = packed.tag_offset,
        .reserved_1 = 0,
        .n_fields = packed.n_fields,
        .reserved_2 = 0,
      };
      emit_op(op);
      continue;
    }

    case opcode_t::op_sta: {
      const auto packed = read_op<op_sta_t>();
      const cold::op_sta_t op = {
        .as_handler = {.as_id = std::to_underlying(packed.opcode)},
      };
      emit_op(op);
      continue;
    }

    case opcode_t::op_jmp: {
      const auto packed = read_op<op_jmp_t>();
      const cold::op_jmp_t op = {
        .as_handler = {.as_id = std::to_underlying(packed.opcode)},
        .target = packed.target,
        .reserved_1 = 0,
      };
      emit_op(op);
      continue;
    }

    case opcode_t::op_end: {
      const auto packed = read_op<op_end_t>();
      const cold::op_end_t op = {
        .as_handler = {.as_id = std::to_underlying(packed.opcode)},
      };
      emit_op(op);
      continue;
    }

    case opcode_t::op_drop: {
      const auto packed = read_op<op_drop_t>();
      const cold::op_drop_t op = {
        .as_handler = {.as_id = std::to_underlying(packed.opcode)},
      };
      emit_op(op);
      continue;
    }

    case opcode_t::op_dup: {
      const auto packed = read_op<op_dup_t>();
      const cold::op_dup_t op = {
        .as_handler = {.as_id = std::to_underlying(packed.opcode)},
      };
      emit_op(op);
      continue;
    }

    case opcode_t::op_swap: {
      const auto packed = read_op<op_swap_t>();
      const cold::op_swap_t op = {
        .as_handler = {.as_id = std::to_underlying(packed.opcode)},
      };
      emit_op(op);
      continue;
    }

    case opcode_t::op_elem: {
      const auto packed = read_op<op_elem_t>();
      const cold::op_elem_t op = {
        .as_handler = {.as_id = std::to_underlying(packed.opcode)},
      };
      emit_op(op);
      continue;
    }

    case opcode_t::op_ld_glo: {
      const auto packed = read_op<op_ld_glo_t>();
      const cold::op_ld_glo_t op = {
        .as_handler = {.as_id = std::to_underlying(packed.opcode)},
        .index = packed.index,
        .reserved_1 = 0,
      };
      emit_op(op);
      continue;
    }

    case opcode_t::op_ld_loc: {
      const auto packed = read_op<op_ld_loc_t>();
      const cold::op_ld_loc_t op = {
        .as_handler = {.as_id = std::to_underlying(packed.opcode)},
        .index = packed.index,
        .reserved_1 = 0,
      };
      emit_op(op);
      continue;
    }

    case opcode_t::op_ld_arg: {
      const auto packed = read_op<op_ld_arg_t>();
      const cold::op_ld_arg_t op = {
        .as_handler = {.as_id = std::to_underlying(packed.opcode)},
        .index = packed.index,
        .reserved_1 = 0,
      };
      emit_op(op);
      continue;
    }

    case opcode_t::op_ld_clo: {
      const auto packed = read_op<op_ld_clo_t>();
      const cold::op_ld_clo_t op = {
        .as_handler = {.as_id = std::to_underlying(packed.opcode)},
        .index = packed.index,
        .reserved_1 = 0,
      };
      emit_op(op);
      continue;
    }

    case opcode_t::op_st_glo: {
      const auto packed = read_op<op_st_glo_t>();
      const cold::op_st_glo_t op = {
        .as_handler = {.as_id = std::to_underlying(packed.opcode)},
        .index = packed.index,
        .reserved_1 = 0,
      };
      emit_op(op);
      continue;
    }

    case opcode_t::op_st_loc: {
      const auto packed = read_op<op_st_loc_t>();
      const cold::op_st_loc_t op = {
        .as_handler = {.as_id = std::to_underlying(packed.opcode)},
        .index = packed.index,
        .reserved_1 = 0,
      };
      emit_op(op);
      continue;
    }

    case opcode_t::op_st_arg: {
      const auto packed = read_op<op_st_arg_t>();
      const cold::op_st_arg_t op = {
        .as_handler = {.as_id = std::to_underlying(packed.opcode)},
        .index = packed.index,
        .reserved_1 = 0,
      };
      emit_op(op);
      continue;
    }

    case opcode_t::op_st_clo: {
      const auto packed = read_op<op_st_clo_t>();
      const cold::op_st_clo_t op = {
        .as_handler = {.as_id = std::to_underlying(packed.opcode)},
        .index = packed.index,
        .reserved_1 = 0,
      };
      emit_op(op);
      continue;
    }

    case opcode_t::op_cjmp_z: {
      const auto packed = read_op<op_cjmp_z_t>();
      const cold::op_cjmp_z_t op = {
        .as_handler = {.as_id = std::to_underlying(packed.opcode)},
        .target = packed.target,
        .reserved_1 = 0,
      };
      emit_op(op);
      continue;
    }

    case opcode_t::op_cjmp_nz: {
      const auto packed = read_op<op_cjmp_nz_t>();
      const cold::op_cjmp_nz_t op = {
        .as_handler = {.as_id = std::to_underlying(packed.opcode)},
        .target = packed.target,
        .reserved_1 = 0,
      };
      emit_op(op);
      continue;
    }

    case opcode_t::op_begin: {
      const auto packed = read_op<op_begin_t>();
      const cold::op_begin_t op = {
        .as_handler = {.as_id = std::to_underlying(packed.opcode)},
        .n_args = packed.n_args,
        .n_locals = packed.n_locals,
      };
      emit_op(op);
      continue;
    }

    case opcode_t::op_begin_closure: {
      const auto packed = read_op<op_begin_closure_t>();
      const cold::op_begin_closure_t op = {
        .as_handler = {.as_id = std::to_underlying(packed.opcode)},
        .n_args = packed.n_args,
        .n_locals = packed.n_locals,
        .n_captures = packed.n_captures,
        .reserved_1 = 0,
      };
      emit_op(op);
      continue;
    }

    case opcode_t::op_closure: {
      const auto packed = read_op<op_closure_header_t>();
      const auto count = packed.n_captured;
      LAMAVM_EXPECT_B(0, <=, count);

      const cold::op_closure_header_t op = {
        .as_handler = {.as_id = std::to_underlying(packed.opcode)},
        .target = packed.target,
        .reserved_1 = 0,
        .reserved_2 = 0,
        .n_captured = packed.n_captured,
      };
      emit_op(op);

      for (std::int32_t i = 0; i < count; ++i) {
        const auto packed_capture = read_capture();
        const cold::closure_capture_t capture{
          .kind = std::to_underlying(packed_capture.kind),
          .index = packed_capture.index,
        };
        emit_capture(capture);
      }

      continue;
    }

    case opcode_t::op_callc: {
      const auto packed = read_op<op_callc_t>();
      const cold::op_callc_t op = {
        .as_handler = {.as_id = std::to_underlying(packed.opcode)},
        .n_args = packed.n_args,
        .reserved_1 = 0,
      };
      emit_op(op);
      continue;
    }

    case opcode_t::op_call: {
      const auto packed = read_op<op_call_t>();
      const cold::op_call_t op = {
        .as_handler = {.as_id = std::to_underlying(packed.opcode)},
        .target = packed.target,
        .reserved_1 = 0,
        .reserved_2 = 0,
        .n_args = packed.n_args,
      };
      emit_op(op);
      continue;
    }

    case opcode_t::op_tag: {
      const auto packed = read_op<op_tag_t>();
      const cold::op_tag_t op = {
        .as_handler = {.as_id = std::to_underlying(packed.opcode)},
        .tag_offset = packed.tag_offset,
        .reserved_1 = 0,
        .n_fields = packed.n_fields,
        .reserved_2 = 0,
      };
      emit_op(op);
      continue;
    }

    case opcode_t::op_array: {
      const auto packed = read_op<op_array_t>();
      const cold::op_array_t op = {
        .as_handler = {.as_id = std::to_underlying(packed.opcode)},
        .n_elements = packed.n_elements,
        .reserved_1 = 0,
      };
      emit_op(op);
      continue;
    }

    case opcode_t::op_fail: {
      const auto packed = read_op<op_fail_t>();
      const cold::op_fail_t op = {
        .as_handler = {.as_id = std::to_underlying(packed.opcode)},
        .line = packed.line,
        .column = packed.column,
      };
      emit_op(op);
      continue;
    }

    case opcode_t::op_fail_keep: {
      const auto packed = read_op<op_fail_keep_t>();
      const cold::op_fail_keep_t op = {
        .as_handler = {.as_id = std::to_underlying(packed.opcode)},
        .line = packed.line,
        .column = packed.column,
      };
      emit_op(op);
      continue;
    }

    case opcode_t::op_line: {
      const auto packed = read_op<op_line_t>();
      const cold::op_line_t op = {
        .as_handler = {.as_id = std::to_underlying(packed.opcode)},
        .line = packed.line,
        .reserved_1 = 0,
      };
      emit_op(op);
      continue;
    }

    case opcode_t::op_patt_str_cmp: {
      const auto packed = read_op<op_patt_str_cmp_t>();
      const cold::op_patt_str_cmp_t op = {
        .as_handler = {.as_id = std::to_underlying(packed.opcode)},
      };
      emit_op(op);
      continue;
    }

    case opcode_t::op_patt_string: {
      const auto packed = read_op<op_patt_string_t>();
      const cold::op_patt_string_t op = {
        .as_handler = {.as_id = std::to_underlying(packed.opcode)},
      };
      emit_op(op);
      continue;
    }

    case opcode_t::op_patt_array: {
      const auto packed = read_op<op_patt_array_t>();
      const cold::op_patt_array_t op = {
        .as_handler = {.as_id = std::to_underlying(packed.opcode)},
      };
      emit_op(op);
      continue;
    }

    case opcode_t::op_patt_sexp: {
      const auto packed = read_op<op_patt_sexp_t>();
      const cold::op_patt_sexp_t op = {
        .as_handler = {.as_id = std::to_underlying(packed.opcode)},
      };
      emit_op(op);
      continue;
    }

    case opcode_t::op_patt_boxed: {
      const auto packed = read_op<op_patt_boxed_t>();
      const cold::op_patt_boxed_t op = {
        .as_handler = {.as_id = std::to_underlying(packed.opcode)},
      };
      emit_op(op);
      continue;
    }

    case opcode_t::op_patt_unboxed: {
      const auto packed = read_op<op_patt_unboxed_t>();
      const cold::op_patt_unboxed_t op = {
        .as_handler = {.as_id = std::to_underlying(packed.opcode)},
      };
      emit_op(op);
      continue;
    }

    case opcode_t::op_patt_closure: {
      const auto packed = read_op<op_patt_closure_t>();
      const cold::op_patt_closure_t op = {
        .as_handler = {.as_id = std::to_underlying(packed.opcode)},
      };
      emit_op(op);
      continue;
    }

    case opcode_t::op_barray: {
      const auto packed = read_op<op_barray_t>();
      const cold::op_barray_t op = {
        .as_handler = {.as_id = std::to_underlying(packed.opcode)},
        .n_elements = packed.n_elements,
        .reserved_1 = 0,
      };
      emit_op(op);
      continue;
    }

    case opcode_t::op_eof: {
      const auto packed = read_op<op_eof_t>();
      const cold::op_eof_t op = {
        .as_handler = {.as_id = std::to_underlying(packed.opcode)},
      };
      emit_op(op);
      LAMAVM_EXPECT_B(in.peek(), ==, std::char_traits<char>::eof());
      return;
    }
    }

    LAMAVM_EXPECT_C(
      false,
      std::format("unknown bytecode opcode 0x{:02x}", std::to_underlying(opcode)));
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

  const auto* const end = u.code_section.data() + u.code_section.size();
  for (auto* pc = u.code_section.data(); pc != end;) {
    const auto opcode = static_cast<opcode_t>(pc->as_handler.as_id);

    switch (opcode) {
    case opcode_t::op_add:
      pc += op::word_count<cold::op_add_t>();
      break;
    case opcode_t::op_sub:
      pc += op::word_count<cold::op_sub_t>();
      break;
    case opcode_t::op_mul:
      pc += op::word_count<cold::op_mul_t>();
      break;
    case opcode_t::op_div:
      pc += op::word_count<cold::op_div_t>();
      break;
    case opcode_t::op_mod:
      pc += op::word_count<cold::op_mod_t>();
      break;
    case opcode_t::op_lt:
      pc += op::word_count<cold::op_lt_t>();
      break;
    case opcode_t::op_le:
      pc += op::word_count<cold::op_le_t>();
      break;
    case opcode_t::op_gt:
      pc += op::word_count<cold::op_gt_t>();
      break;
    case opcode_t::op_ge:
      pc += op::word_count<cold::op_ge_t>();
      break;
    case opcode_t::op_eq:
      pc += op::word_count<cold::op_eq_t>();
      break;
    case opcode_t::op_ne:
      pc += op::word_count<cold::op_ne_t>();
      break;
    case opcode_t::op_and:
      pc += op::word_count<cold::op_and_t>();
      break;
    case opcode_t::op_or:
      pc += op::word_count<cold::op_or_t>();
      break;
    case opcode_t::op_const:
      pc += op::word_count<cold::op_const_t>();
      break;
    case opcode_t::op_string:
      pc += op::word_count<cold::op_string_t>();
      break;
    case opcode_t::op_sexp:
      pc += op::word_count<cold::op_sexp_t>();
      break;
    case opcode_t::op_sta:
      pc += op::word_count<cold::op_sta_t>();
      break;
    case opcode_t::op_jmp: {
      auto& instruction = *cold::op_jmp_t::reinterpret(pc);
      LAMAVM_EXPECT_B(instruction.reserved_1, ==, 0);

      instruction.target = word_offset(instruction.target);
      pc += op::word_count<cold::op_jmp_t>();
      break;
    }

    case opcode_t::op_end:
      pc += op::word_count<cold::op_end_t>();
      break;
    case opcode_t::op_drop:
      pc += op::word_count<cold::op_drop_t>();
      break;
    case opcode_t::op_dup:
      pc += op::word_count<cold::op_dup_t>();
      break;
    case opcode_t::op_swap:
      pc += op::word_count<cold::op_swap_t>();
      break;
    case opcode_t::op_elem:
      pc += op::word_count<cold::op_elem_t>();
      break;
    case opcode_t::op_ld_glo:
      pc += op::word_count<cold::op_ld_glo_t>();
      break;
    case opcode_t::op_ld_loc:
      pc += op::word_count<cold::op_ld_loc_t>();
      break;
    case opcode_t::op_ld_arg:
      pc += op::word_count<cold::op_ld_arg_t>();
      break;
    case opcode_t::op_ld_clo:
      pc += op::word_count<cold::op_ld_clo_t>();
      break;
    case opcode_t::op_st_glo:
      pc += op::word_count<cold::op_st_glo_t>();
      break;
    case opcode_t::op_st_loc:
      pc += op::word_count<cold::op_st_loc_t>();
      break;
    case opcode_t::op_st_arg:
      pc += op::word_count<cold::op_st_arg_t>();
      break;
    case opcode_t::op_st_clo:
      pc += op::word_count<cold::op_st_clo_t>();
      break;
    case opcode_t::op_cjmp_z: {
      auto& instruction = *cold::op_cjmp_z_t::reinterpret(pc);
      LAMAVM_EXPECT_B(instruction.reserved_1, ==, 0);

      instruction.target = word_offset(instruction.target);
      pc += op::word_count<cold::op_cjmp_z_t>();
      break;
    }

    case opcode_t::op_cjmp_nz: {
      auto& instruction = *cold::op_cjmp_nz_t::reinterpret(pc);
      LAMAVM_EXPECT_B(instruction.reserved_1, ==, 0);

      instruction.target = word_offset(instruction.target);
      pc += op::word_count<cold::op_cjmp_nz_t>();
      break;
    }

    case opcode_t::op_begin:
      pc += op::word_count<cold::op_begin_t>();
      break;
    case opcode_t::op_begin_closure:
      pc += op::word_count<cold::op_begin_closure_t>();
      break;
    case opcode_t::op_closure: {
      const auto closure = cold::op_closure_view_t::reinterpret(pc);
      auto& header = *closure.header;
      LAMAVM_EXPECT_B(header.reserved_1, ==, 0);
      LAMAVM_EXPECT_B(header.reserved_2, ==, 0);

      if (header.target >= 0) {
        header.target = word_offset(header.target);
      }

      pc += cold::word_count(closure);
      break;
    }

    case opcode_t::op_callc:
      pc += op::word_count<cold::op_callc_t>();
      break;
    case opcode_t::op_call: {
      auto& instruction = *cold::op_call_t::reinterpret(pc);
      LAMAVM_EXPECT_B(instruction.reserved_1, ==, 0);

      if (instruction.target >= 0) {
        instruction.target = word_offset(instruction.target);
      }

      pc += op::word_count<cold::op_call_t>();
      break;
    }

    case opcode_t::op_tag:
      pc += op::word_count<cold::op_tag_t>();
      break;
    case opcode_t::op_array:
      pc += op::word_count<cold::op_array_t>();
      break;
    case opcode_t::op_fail:
      pc += op::word_count<cold::op_fail_t>();
      break;
    case opcode_t::op_fail_keep:
      pc += op::word_count<cold::op_fail_keep_t>();
      break;
    case opcode_t::op_line:
      pc += op::word_count<cold::op_line_t>();
      break;
    case opcode_t::op_patt_str_cmp:
      pc += op::word_count<cold::op_patt_str_cmp_t>();
      break;
    case opcode_t::op_patt_string:
      pc += op::word_count<cold::op_patt_string_t>();
      break;
    case opcode_t::op_patt_array:
      pc += op::word_count<cold::op_patt_array_t>();
      break;
    case opcode_t::op_patt_sexp:
      pc += op::word_count<cold::op_patt_sexp_t>();
      break;
    case opcode_t::op_patt_boxed:
      pc += op::word_count<cold::op_patt_boxed_t>();
      break;
    case opcode_t::op_patt_unboxed:
      pc += op::word_count<cold::op_patt_unboxed_t>();
      break;
    case opcode_t::op_patt_closure:
      pc += op::word_count<cold::op_patt_closure_t>();
      break;
    case opcode_t::op_barray:
      pc += op::word_count<cold::op_barray_t>();
      break;
    case opcode_t::op_eof:
      pc += op::word_count<cold::op_eof_t>();
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
         (u.code_section.size() * sizeof(decltype(u.code_section)::value_type)) +
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

LAMAVM_FORMAT_DEFINE(enum lamavm::bc::public_symbol::kind, x, ctx) {
  using enum lamavm::bc::public_symbol::kind;

  switch (x) {
  case lamavm::bc::public_symbol::kind::function:
    return LAMAVM_FORMAT_DERIVE(ctx, "function");
  case lamavm::bc::public_symbol::kind::global:
    return LAMAVM_FORMAT_DERIVE(ctx, "global");
  }

  return LAMAVM_FORMAT_DERIVE(ctx, "{:x}", static_cast<unsigned int>(x));
}
