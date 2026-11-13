#pragma once

#include "core/c.hpp"
#include "core/format.hpp"

#include <array>
#include <cstdint>

namespace lamavm::bc {

struct [[gnu::packed]] header_t {
  std::array<char, 4> magic;
  std::int32_t version;
  std::int32_t string_table_size;
  std::int32_t globals_count;
  std::int32_t imports_count;
  std::int32_t public_symbols_count;
};

struct [[gnu::packed]] import_t {
  std::int32_t name_offset;
};

struct [[gnu::packed]] public_symbol_t {
  std::int32_t name_offset;
  std::int32_t code_offset;
  std::int8_t flag;
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

struct [[gnu::packed]] op_begin_t {
  opcode_t opcode;
  std::int32_t n_args;
  std::int32_t n_locals;
};

struct [[gnu::packed]] op_const_t {
  opcode_t opcode;
  std::int32_t value;
};

struct [[gnu::packed]] op_end_t {
  opcode_t opcode;
};

struct [[gnu::packed]] op_eof_t {
  opcode_t opcode;
};

struct [[gnu::packed]] op_add_t {
  opcode_t opcode;
};

struct [[gnu::packed]] op_sub_t {
  opcode_t opcode;
};

struct [[gnu::packed]] op_mul_t {
  opcode_t opcode;
};

struct [[gnu::packed]] op_div_t {
  opcode_t opcode;
};

struct [[gnu::packed]] op_mod_t {
  opcode_t opcode;
};

struct [[gnu::packed]] op_lt_t {
  opcode_t opcode;
};

struct [[gnu::packed]] op_le_t {
  opcode_t opcode;
};

struct [[gnu::packed]] op_gt_t {
  opcode_t opcode;
};

struct [[gnu::packed]] op_ge_t {
  opcode_t opcode;
};

struct [[gnu::packed]] op_eq_t {
  opcode_t opcode;
};

struct [[gnu::packed]] op_ne_t {
  opcode_t opcode;
};

struct [[gnu::packed]] op_and_t {
  opcode_t opcode;
};

struct [[gnu::packed]] op_or_t {
  opcode_t opcode;
};

struct [[gnu::packed]] op_string_t {
  opcode_t opcode;
  std::int32_t string_offset;
};

struct [[gnu::packed]] op_jmp_t {
  opcode_t opcode;
  std::int32_t target;
};

struct [[gnu::packed]] op_cjmp_z_t {
  opcode_t opcode;
  std::int32_t target;
};

struct [[gnu::packed]] op_cjmp_nz_t {
  opcode_t opcode;
  std::int32_t target;
};

struct [[gnu::packed]] op_drop_t {
  opcode_t opcode;
};

struct [[gnu::packed]] op_dup_t {
  opcode_t opcode;
};

struct [[gnu::packed]] op_swap_t {
  opcode_t opcode;
};

struct [[gnu::packed]] op_call_t {
  opcode_t opcode;
  std::int32_t target;
  std::int32_t n_args;
};

struct [[gnu::packed]] op_ld_loc_t {
  opcode_t opcode;
  std::int32_t index;
};

struct [[gnu::packed]] op_ld_arg_t {
  opcode_t opcode;
  std::int32_t index;
};

struct [[gnu::packed]] op_st_loc_t {
  opcode_t opcode;
  std::int32_t index;
};

struct [[gnu::packed]] op_st_arg_t {
  opcode_t opcode;
  std::int32_t index;
};

struct [[gnu::packed]] op_ld_glo_t {
  opcode_t opcode;
  std::int32_t index;
};

struct [[gnu::packed]] op_st_glo_t {
  opcode_t opcode;
  std::int32_t index;
};

struct [[gnu::packed]] op_line_t {
  opcode_t opcode;
  std::int32_t line;
};

static_assert(ctype<header_t>);
static_assert(ctype<import_t>);
static_assert(ctype<public_symbol_t>);
static_assert(ctype<op_begin_t>);
static_assert(ctype<op_const_t>);
static_assert(ctype<op_end_t>);
static_assert(ctype<op_eof_t>);
static_assert(ctype<op_add_t>);
static_assert(ctype<op_sub_t>);
static_assert(ctype<op_mul_t>);
static_assert(ctype<op_div_t>);
static_assert(ctype<op_mod_t>);
static_assert(ctype<op_lt_t>);
static_assert(ctype<op_le_t>);
static_assert(ctype<op_gt_t>);
static_assert(ctype<op_ge_t>);
static_assert(ctype<op_eq_t>);
static_assert(ctype<op_ne_t>);
static_assert(ctype<op_and_t>);
static_assert(ctype<op_or_t>);
static_assert(ctype<op_string_t>);
static_assert(ctype<op_jmp_t>);
static_assert(ctype<op_cjmp_z_t>);
static_assert(ctype<op_cjmp_nz_t>);
static_assert(ctype<op_drop_t>);
static_assert(ctype<op_dup_t>);
static_assert(ctype<op_swap_t>);
static_assert(ctype<op_call_t>);
static_assert(ctype<op_ld_loc_t>);
static_assert(ctype<op_ld_arg_t>);
static_assert(ctype<op_st_loc_t>);
static_assert(ctype<op_st_arg_t>);
static_assert(ctype<op_ld_glo_t>);
static_assert(ctype<op_st_glo_t>);
static_assert(ctype<op_line_t>);

} // namespace lamavm::bc

LAMAVM_FORMAT_DECLARE(lamavm::bc::import_t, x, ctx);
LAMAVM_FORMAT_DECLARE(lamavm::bc::public_symbol_t, x, ctx);
LAMAVM_FORMAT_DECLARE(lamavm::bc::opcode_t, x, ctx);
LAMAVM_FORMAT_DECLARE(lamavm::bc::op_begin_t, x, ctx);
LAMAVM_FORMAT_DECLARE(lamavm::bc::op_const_t, x, ctx);
LAMAVM_FORMAT_DECLARE(lamavm::bc::op_end_t, x, ctx);
LAMAVM_FORMAT_DECLARE(lamavm::bc::op_eof_t, x, ctx);
LAMAVM_FORMAT_DECLARE(lamavm::bc::op_add_t, x, ctx);
LAMAVM_FORMAT_DECLARE(lamavm::bc::op_sub_t, x, ctx);
LAMAVM_FORMAT_DECLARE(lamavm::bc::op_mul_t, x, ctx);
LAMAVM_FORMAT_DECLARE(lamavm::bc::op_div_t, x, ctx);
LAMAVM_FORMAT_DECLARE(lamavm::bc::op_mod_t, x, ctx);
LAMAVM_FORMAT_DECLARE(lamavm::bc::op_lt_t, x, ctx);
LAMAVM_FORMAT_DECLARE(lamavm::bc::op_le_t, x, ctx);
LAMAVM_FORMAT_DECLARE(lamavm::bc::op_gt_t, x, ctx);
LAMAVM_FORMAT_DECLARE(lamavm::bc::op_ge_t, x, ctx);
LAMAVM_FORMAT_DECLARE(lamavm::bc::op_eq_t, x, ctx);
LAMAVM_FORMAT_DECLARE(lamavm::bc::op_ne_t, x, ctx);
LAMAVM_FORMAT_DECLARE(lamavm::bc::op_and_t, x, ctx);
LAMAVM_FORMAT_DECLARE(lamavm::bc::op_or_t, x, ctx);
LAMAVM_FORMAT_DECLARE(lamavm::bc::op_string_t, x, ctx);
LAMAVM_FORMAT_DECLARE(lamavm::bc::op_jmp_t, x, ctx);
LAMAVM_FORMAT_DECLARE(lamavm::bc::op_cjmp_z_t, x, ctx);
LAMAVM_FORMAT_DECLARE(lamavm::bc::op_cjmp_nz_t, x, ctx);
LAMAVM_FORMAT_DECLARE(lamavm::bc::op_drop_t, x, ctx);
LAMAVM_FORMAT_DECLARE(lamavm::bc::op_dup_t, x, ctx);
LAMAVM_FORMAT_DECLARE(lamavm::bc::op_swap_t, x, ctx);
LAMAVM_FORMAT_DECLARE(lamavm::bc::op_call_t, x, ctx);
LAMAVM_FORMAT_DECLARE(lamavm::bc::op_ld_loc_t, x, ctx);
LAMAVM_FORMAT_DECLARE(lamavm::bc::op_ld_arg_t, x, ctx);
LAMAVM_FORMAT_DECLARE(lamavm::bc::op_st_loc_t, x, ctx);
LAMAVM_FORMAT_DECLARE(lamavm::bc::op_st_arg_t, x, ctx);
LAMAVM_FORMAT_DECLARE(lamavm::bc::op_ld_glo_t, x, ctx);
LAMAVM_FORMAT_DECLARE(lamavm::bc::op_st_glo_t, x, ctx);
LAMAVM_FORMAT_DECLARE(lamavm::bc::op_line_t, x, ctx);
