#pragma once

#include <array>
#include <cstdint>
#include <iosfwd>

namespace lamavm::bc {

struct [[gnu::packed]] header_t {
  std::array<char, 4> magic = {0};
  std::int32_t version = 0;
  std::int32_t string_table_size = 0;
  std::int32_t globals_count = 0;
  std::int32_t imports_count = 0;
  std::int32_t public_symbols_count = 0;
};

struct [[gnu::packed]] import_t {
  std::int32_t name_offset = 0;
};

struct [[gnu::packed]] public_symbol_t {
  std::int32_t name_offset = 0;
  std::int32_t code_offset = 0;
  bool flag = false;
};

enum class opcode_t : std::uint8_t {
  op_add = 0x01,
  op_sub = 0x02,
  op_mul = 0x03,
  op_div = 0x04,
  op_mod = 0x05,
  op_lt = 0x06,
  op_le = 0x07,
  op_gt = 0x08,
  op_ge = 0x09,
  op_eq = 0x0A,
  op_ne = 0x0B,
  op_and = 0x0C,
  op_or = 0x0D,

  op_const = 0x10,
  op_string = 0x11,
  op_sexp = 0x12,
  op_sta = 0x14,
  op_jmp = 0x15,
  op_end = 0x16,
  op_drop = 0x18,
  op_dup = 0x19,
  op_swap = 0x1A,
  op_elem = 0x1B,

  op_ld_glo = 0x20,
  op_ld_loc = 0x21,
  op_ld_arg = 0x22,
  op_ld_clo = 0x23,

  op_st_glo = 0x40,
  op_st_loc = 0x41,
  op_st_arg = 0x42,
  op_st_clo = 0x43,

  op_cjmp_z = 0x50,
  op_cjmp_nz = 0x51,
  op_begin = 0x52,
  op_begin_closure = 0x53,
  op_closure = 0x54,
  op_callc = 0x55,
  op_call = 0x56,
  op_tag = 0x57,
  op_array = 0x58,
  op_fail = 0x59,
  op_fail_keep = 0x5A,
  op_line = 0x5B,

  op_patt_str_cmp = 0x60,
  op_patt_string = 0x61,
  op_patt_array = 0x62,
  op_patt_sexp = 0x63,
  op_patt_boxed = 0x64,
  op_patt_unboxed = 0x65,
  op_patt_closure = 0x66,

  op_barray = 0x74,
  op_eof = 0xFF,
};

std::ostream& operator<<(std::ostream& os, opcode_t opcode);

} // namespace lamavm::bc
