#pragma once

#include <array>
#include <cstdint>

namespace lamavm::bc {

enum class opcode : std::uint8_t {
  add = 0x01,
  sub = 0x02,
  mul = 0x03,
  div = 0x04,
  mod = 0x05,
  lt = 0x06,
  le = 0x07,
  gt = 0x08,
  ge = 0x09,
  eq = 0x0A,
  ne = 0x0B,
  logical_and = 0x0C,
  logical_or = 0x0D,
  constant = 0x10,
  string = 0x11,
  sexp = 0x12,
  sti = 0x13,
  sta = 0x14,
  jmp = 0x15,
  end = 0x16,
  ret = 0x17,
  drop = 0x18,
  dup = 0x19,
  swap = 0x1A,
  elem = 0x1B,
  ld_glo = 0x20,
  ld_loc = 0x21,
  ld_arg = 0x22,
  ld_clo = 0x23,
  lda_glo = 0x30,
  lda_loc = 0x31,
  lda_arg = 0x32,
  lda_clo = 0x33,
  st_glo = 0x40,
  st_loc = 0x41,
  st_arg = 0x42,
  st_clo = 0x43,
  cjmp_z = 0x50,
  cjmp_nz = 0x51,
  begin = 0x52,
  begin_closure = 0x53,
  closure = 0x54,
  callc = 0x55,
  call = 0x56,
  tag = 0x57,
  array = 0x58,
  fail = 0x59,
  fail_keep = 0x5A,
  line = 0x5B,
  patt_str_cmp = 0x60,
  patt_string = 0x61,
  patt_array = 0x62,
  patt_sexp = 0x63,
  patt_boxed = 0x64,
  patt_unboxed = 0x65,
  patt_closure = 0x66,
  barray = 0x74,
  eof = 0xFF,
};

struct [[gnu::packed]] header {
  std::array<char, 4> magic{};
  std::int32_t version{};
  std::int32_t string_table_size{};
  std::int32_t globals_count{};
  std::int32_t imports_count{};
  std::int32_t public_symbols_count{};
};

struct [[gnu::packed]] import {
  std::int32_t name_offset{};
};

struct [[gnu::packed]] public_symbol {
  enum class flag : std::uint8_t {
    function = 0,
    global,
  };

  std::int32_t name_offset{};
  std::int32_t code_offset{};
  flag flag{};
};

struct [[gnu::packed]] add {
  static constexpr bc::opcode code = bc::opcode::add;

  bc::opcode opcode = code;
};

struct [[gnu::packed]] sub {
  static constexpr bc::opcode code = bc::opcode::sub;

  bc::opcode opcode = code;
};

struct [[gnu::packed]] mul {
  static constexpr bc::opcode code = bc::opcode::mul;

  bc::opcode opcode = code;
};

struct [[gnu::packed]] div {
  static constexpr bc::opcode code = bc::opcode::div;

  bc::opcode opcode = code;
};

struct [[gnu::packed]] mod {
  static constexpr bc::opcode code = bc::opcode::mod;

  bc::opcode opcode = code;
};

struct [[gnu::packed]] lt {
  static constexpr bc::opcode code = bc::opcode::lt;

  bc::opcode opcode = code;
};

struct [[gnu::packed]] le {
  static constexpr bc::opcode code = bc::opcode::le;

  bc::opcode opcode = code;
};

struct [[gnu::packed]] gt {
  static constexpr bc::opcode code = bc::opcode::gt;

  bc::opcode opcode = code;
};

struct [[gnu::packed]] ge {
  static constexpr bc::opcode code = bc::opcode::ge;

  bc::opcode opcode = code;
};

struct [[gnu::packed]] eq {
  static constexpr bc::opcode code = bc::opcode::eq;

  bc::opcode opcode = code;
};

struct [[gnu::packed]] ne {
  static constexpr bc::opcode code = bc::opcode::ne;

  bc::opcode opcode = code;
};

struct [[gnu::packed]] logical_and {
  static constexpr bc::opcode code = bc::opcode::logical_and;

  bc::opcode opcode = code;
};

struct [[gnu::packed]] logical_or {
  static constexpr bc::opcode code = bc::opcode::logical_or;

  bc::opcode opcode = code;
};

struct [[gnu::packed]] constant {
  static constexpr bc::opcode code = bc::opcode::constant;

  bc::opcode opcode = code;
  std::int32_t value{};
};

struct [[gnu::packed]] string {
  static constexpr bc::opcode code = bc::opcode::string;

  bc::opcode opcode = code;
  std::int32_t string_offset{};
};

struct [[gnu::packed]] sexp {
  static constexpr bc::opcode code = bc::opcode::sexp;

  bc::opcode opcode = code;
  std::int32_t tag_offset{};
  std::int32_t n_fields{};
};

struct [[gnu::packed]] sti {
  static constexpr bc::opcode code = bc::opcode::sti;

  bc::opcode opcode = code;
};

struct [[gnu::packed]] sta {
  static constexpr bc::opcode code = bc::opcode::sta;

  bc::opcode opcode = code;
};

struct [[gnu::packed]] jmp {
  static constexpr bc::opcode code = bc::opcode::jmp;

  bc::opcode opcode = code;
  std::int32_t target{};
};

struct [[gnu::packed]] end {
  static constexpr bc::opcode code = bc::opcode::end;

  bc::opcode opcode = code;
};

struct [[gnu::packed]] ret {
  static constexpr bc::opcode code = bc::opcode::ret;

  bc::opcode opcode = code;
};

struct [[gnu::packed]] drop {
  static constexpr bc::opcode code = bc::opcode::drop;

  bc::opcode opcode = code;
};

struct [[gnu::packed]] dup {
  static constexpr bc::opcode code = bc::opcode::dup;

  bc::opcode opcode = code;
};

struct [[gnu::packed]] swap {
  static constexpr bc::opcode code = bc::opcode::swap;

  bc::opcode opcode = code;
};

struct [[gnu::packed]] elem {
  static constexpr bc::opcode code = bc::opcode::elem;

  bc::opcode opcode = code;
};

struct [[gnu::packed]] ld_glo {
  static constexpr bc::opcode code = bc::opcode::ld_glo;

  bc::opcode opcode = code;
  std::int32_t operand{};
};

struct [[gnu::packed]] ld_loc {
  static constexpr bc::opcode code = bc::opcode::ld_loc;

  bc::opcode opcode = code;
  std::int32_t operand{};
};

struct [[gnu::packed]] ld_arg {
  static constexpr bc::opcode code = bc::opcode::ld_arg;

  bc::opcode opcode = code;
  std::int32_t operand{};
};

struct [[gnu::packed]] ld_clo {
  static constexpr bc::opcode code = bc::opcode::ld_clo;

  bc::opcode opcode = code;
  std::int32_t operand{};
};

struct [[gnu::packed]] lda_glo {
  static constexpr bc::opcode code = bc::opcode::lda_glo;

  bc::opcode opcode = code;
  std::int32_t operand{};
};

struct [[gnu::packed]] lda_loc {
  static constexpr bc::opcode code = bc::opcode::lda_loc;

  bc::opcode opcode = code;
  std::int32_t operand{};
};

struct [[gnu::packed]] lda_arg {
  static constexpr bc::opcode code = bc::opcode::lda_arg;

  bc::opcode opcode = code;
  std::int32_t operand{};
};

struct [[gnu::packed]] lda_clo {
  static constexpr bc::opcode code = bc::opcode::lda_clo;

  bc::opcode opcode = code;
  std::int32_t operand{};
};

struct [[gnu::packed]] st_glo {
  static constexpr bc::opcode code = bc::opcode::st_glo;

  bc::opcode opcode = code;
  std::int32_t operand{};
};

struct [[gnu::packed]] st_loc {
  static constexpr bc::opcode code = bc::opcode::st_loc;

  bc::opcode opcode = code;
  std::int32_t operand{};
};

struct [[gnu::packed]] st_arg {
  static constexpr bc::opcode code = bc::opcode::st_arg;

  bc::opcode opcode = code;
  std::int32_t operand{};
};

struct [[gnu::packed]] st_clo {
  static constexpr bc::opcode code = bc::opcode::st_clo;

  bc::opcode opcode = code;
  std::int32_t operand{};
};

struct [[gnu::packed]] cjmp_z {
  static constexpr bc::opcode code = bc::opcode::cjmp_z;

  bc::opcode opcode = code;
  std::int32_t target{};
};

struct [[gnu::packed]] cjmp_nz {
  static constexpr bc::opcode code = bc::opcode::cjmp_nz;

  bc::opcode opcode = code;
  std::int32_t target{};
};

struct [[gnu::packed]] begin {
  static constexpr bc::opcode code = bc::opcode::begin;

  bc::opcode opcode = code;
  std::int32_t n_args{};
  std::int32_t n_locals{};
};

struct [[gnu::packed]] begin_closure {
  static constexpr bc::opcode code = bc::opcode::begin_closure;

  bc::opcode opcode = code;
  std::int32_t n_args{};
  std::int32_t n_locals{};
  std::int32_t n_captured{};
};

struct [[gnu::packed]] closure {
  struct [[gnu::packed]] capture {
    enum class kind : std::uint8_t {
      global = 0,
      local = 1,
      argument = 2,
      captured = 3,
    };

    kind kind{};
    std::int32_t index{};
  };

  static constexpr bc::opcode code = bc::opcode::closure;

  bc::opcode opcode = code;
  std::int32_t target{};
  std::int32_t n_captured{};
};

struct [[gnu::packed]] callc {
  static constexpr bc::opcode code = bc::opcode::callc;

  bc::opcode opcode = code;
  std::int32_t n_args{};
};

struct [[gnu::packed]] call {
  static constexpr bc::opcode code = bc::opcode::call;

  bc::opcode opcode = code;
  std::int32_t target{};
  std::int32_t n_args{};
};

struct [[gnu::packed]] tag {
  static constexpr bc::opcode code = bc::opcode::tag;

  bc::opcode opcode = code;
  std::int32_t tag_offset{};
  std::int32_t n_fields{};
};

struct [[gnu::packed]] array {
  static constexpr bc::opcode code = bc::opcode::array;

  bc::opcode opcode = code;
  std::int32_t n{};
};

struct [[gnu::packed]] fail {
  static constexpr bc::opcode code = bc::opcode::fail;

  bc::opcode opcode = code;
  std::int32_t line{};
  std::int32_t col{};
};

struct [[gnu::packed]] fail_keep {
  static constexpr bc::opcode code = bc::opcode::fail_keep;

  bc::opcode opcode = code;
  std::int32_t line{};
  std::int32_t col{};
};

struct [[gnu::packed]] line {
  static constexpr bc::opcode code = bc::opcode::line;

  bc::opcode opcode = code;
  std::int32_t line{};
};

struct [[gnu::packed]] patt_str_cmp {
  static constexpr bc::opcode code = bc::opcode::patt_str_cmp;

  bc::opcode opcode = code;
};

struct [[gnu::packed]] patt_string {
  static constexpr bc::opcode code = bc::opcode::patt_string;

  bc::opcode opcode = code;
};

struct [[gnu::packed]] patt_array {
  static constexpr bc::opcode code = bc::opcode::patt_array;

  bc::opcode opcode = code;
};

struct [[gnu::packed]] patt_sexp {
  static constexpr bc::opcode code = bc::opcode::patt_sexp;

  bc::opcode opcode = code;
};

struct [[gnu::packed]] patt_boxed {
  static constexpr bc::opcode code = bc::opcode::patt_boxed;

  bc::opcode opcode = code;
};

struct [[gnu::packed]] patt_unboxed {
  static constexpr bc::opcode code = bc::opcode::patt_unboxed;

  bc::opcode opcode = code;
};

struct [[gnu::packed]] patt_closure {
  static constexpr bc::opcode code = bc::opcode::patt_closure;

  bc::opcode opcode = code;
};

struct [[gnu::packed]] barray {
  static constexpr bc::opcode code = bc::opcode::barray;

  bc::opcode opcode = code;
  std::int32_t n{};
};

struct [[gnu::packed]] eof {
  static constexpr bc::opcode code = bc::opcode::eof;

  bc::opcode opcode = code;
};

} // namespace lamavm::bc
