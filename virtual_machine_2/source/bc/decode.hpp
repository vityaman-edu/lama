#pragma once

#include "layout.hpp"

#include "core/assert.hpp"

#include <bit>
#include <cstddef>
#include <format>
#include <type_traits>
#include <utility>

namespace lamavm::bc {

namespace detail {

template <typename Op, bool is_safe, typename Byte, typename V>
void decode_op(Byte*& ip, const std::byte* end, V& v) {
  using op_t = std::conditional_t<std::is_const_v<Byte>, const Op, Op>;

  static_assert(alignof(Op) == 1);
  static_assert(std::endian::native == std::endian::little);

  LAMAVM_ASSERT_B(ip, <=, end);
  [[maybe_unused]] const auto tail = static_cast<std::size_t>(end - ip);
  if constexpr (is_safe) {
    LAMAVM_EXPECT_B(sizeof(Op), <=, tail);
  } else {
    LAMAVM_ASSERT_B(sizeof(Op), <=, tail);
  }

  auto& op = *reinterpret_cast<op_t*>(ip);
  ip += sizeof(Op);
  v(op);
}

template <bool is_safe, typename Byte, typename V>
void decode(Byte*& ip, const std::byte* end, V& v) {
  if constexpr (is_safe) {
    LAMAVM_EXPECT(ip != nullptr);
    LAMAVM_EXPECT(end != nullptr);
    LAMAVM_EXPECT(ip < end);
  } else {
    LAMAVM_ASSERT(ip != nullptr);
    LAMAVM_ASSERT(end == nullptr);
    LAMAVM_ASSERT(ip < end);
  }

  const auto opcode = static_cast<opcode_t>(*ip);

  switch (opcode) {
  case opcode_t::op_add:
    return decode_op<op_add_t, is_safe>(ip, end, v);
  case opcode_t::op_sub:
    return decode_op<op_sub_t, is_safe>(ip, end, v);
  case opcode_t::op_mul:
    return decode_op<op_mul_t, is_safe>(ip, end, v);
  case opcode_t::op_div:
    return decode_op<op_div_t, is_safe>(ip, end, v);
  case opcode_t::op_mod:
    return decode_op<op_mod_t, is_safe>(ip, end, v);
  case opcode_t::op_lt:
    return decode_op<op_lt_t, is_safe>(ip, end, v);
  case opcode_t::op_le:
    return decode_op<op_le_t, is_safe>(ip, end, v);
  case opcode_t::op_gt:
    return decode_op<op_gt_t, is_safe>(ip, end, v);
  case opcode_t::op_ge:
    return decode_op<op_ge_t, is_safe>(ip, end, v);
  case opcode_t::op_eq:
    return decode_op<op_eq_t, is_safe>(ip, end, v);
  case opcode_t::op_ne:
    return decode_op<op_ne_t, is_safe>(ip, end, v);
  case opcode_t::op_and:
    return decode_op<op_and_t, is_safe>(ip, end, v);
  case opcode_t::op_or:
    return decode_op<op_or_t, is_safe>(ip, end, v);
  case opcode_t::op_begin:
    return decode_op<op_begin_t, is_safe>(ip, end, v);
  case opcode_t::op_const:
    return decode_op<op_const_t, is_safe>(ip, end, v);
  case opcode_t::op_end:
    return decode_op<op_end_t, is_safe>(ip, end, v);
  case opcode_t::op_eof:
    return decode_op<op_eof_t, is_safe>(ip, end, v);
  case opcode_t::op_string:
    return decode_op<op_string_t, is_safe>(ip, end, v);
  case opcode_t::op_jmp:
    return decode_op<op_jmp_t, is_safe>(ip, end, v);
  case opcode_t::op_cjmp_z:
    return decode_op<op_cjmp_z_t, is_safe>(ip, end, v);
  case opcode_t::op_cjmp_nz:
    return decode_op<op_cjmp_nz_t, is_safe>(ip, end, v);
  case opcode_t::op_drop:
    return decode_op<op_drop_t, is_safe>(ip, end, v);
  case opcode_t::op_dup:
    return decode_op<op_dup_t, is_safe>(ip, end, v);
  case opcode_t::op_swap:
    return decode_op<op_swap_t, is_safe>(ip, end, v);
  case opcode_t::op_call:
    return decode_op<op_call_t, is_safe>(ip, end, v);
  case opcode_t::op_ld_loc:
    return decode_op<op_ld_loc_t, is_safe>(ip, end, v);
  case opcode_t::op_ld_arg:
    return decode_op<op_ld_arg_t, is_safe>(ip, end, v);
  case opcode_t::op_st_loc:
    return decode_op<op_st_loc_t, is_safe>(ip, end, v);
  case opcode_t::op_st_arg:
    return decode_op<op_st_arg_t, is_safe>(ip, end, v);
  case opcode_t::op_ld_glo:
    return decode_op<op_ld_glo_t, is_safe>(ip, end, v);
  case opcode_t::op_st_glo:
    return decode_op<op_st_glo_t, is_safe>(ip, end, v);
  case opcode_t::op_sexp:
  case opcode_t::op_sta:
  case opcode_t::op_elem:
  case opcode_t::op_ld_clo:
  case opcode_t::op_st_clo:
  case opcode_t::op_begin_closure:
  case opcode_t::op_closure:
  case opcode_t::op_callc:
  case opcode_t::op_tag:
  case opcode_t::op_array:
  case opcode_t::op_fail:
  case opcode_t::op_fail_keep:
  case opcode_t::op_line:
  case opcode_t::op_patt_str_cmp:
  case opcode_t::op_patt_string:
  case opcode_t::op_patt_array:
  case opcode_t::op_patt_sexp:
  case opcode_t::op_patt_boxed:
  case opcode_t::op_patt_unboxed:
  case opcode_t::op_patt_closure:
  case opcode_t::op_barray:
    LAMAVM_EXPECT_C(false, std::format("unsupported opcode {}", opcode));
  }

  if constexpr (is_safe) {
    LAMAVM_EXPECT_C(false, std::format("unknown opcode 0x{:02x}", std::to_underlying(opcode)));
  } else {
    LAMAVM_ASSERT_C(false, std::format("unknown opcode 0x{:02x}", std::to_underlying(opcode)));
  }

  std::unreachable();
}

} // namespace detail

template <typename V>
void decode_safe(std::byte*& ip, const std::byte* end, V& v) {
  detail::decode<true>(ip, end, v);
}

template <typename V>
void decode_safe(const std::byte*& ip, const std::byte* end, V& v) {
  detail::decode<true>(ip, end, v);
}

template <typename V>
void decode_fast(std::byte*& ip, V& v) {
  detail::decode<false>(ip, /*end=*/nullptr, v);
}

template <typename V>
void decode_fast(const std::byte*& ip, V& v) {
  detail::decode<false>(ip, /*end=*/nullptr, v);
}

} // namespace lamavm::bc
