#include "decode.hpp"

#include "bc/layout.hpp"
#include "core/assert.hpp"
#include "core/log.hpp"
#include "interpreter/cold/handler.hpp"
#include "op/handler.hpp"
#include "op/word.hpp"

#include <cstddef>
#include <limits>
#include <span>
#include <type_traits>
#include <utility>

namespace lamavm::interpreter {

namespace {

op::handler_fn_t decode(op::handler_id_t handler_id) {
  const auto max = std::numeric_limits<std::underlying_type_t<bc::opcode_t>>::max();
  if (max < handler_id) {
    fatalf("unexpected handler overflow {:x}", handler_id);
  }

  const auto handler_opcode = static_cast<bc::opcode_t>(handler_id);

  switch (handler_opcode) {
  case bc::opcode_t::op_add:
    return cold::op_add;
  case bc::opcode_t::op_sub:
    return cold::op_sub;
  case bc::opcode_t::op_mul:
    return cold::op_mul;
  case bc::opcode_t::op_div:
    return cold::op_div;
  case bc::opcode_t::op_mod:
    return cold::op_mod;
  case bc::opcode_t::op_lt:
    return cold::op_lt;
  case bc::opcode_t::op_le:
    return cold::op_le;
  case bc::opcode_t::op_gt:
    return cold::op_gt;
  case bc::opcode_t::op_ge:
    return cold::op_ge;
  case bc::opcode_t::op_eq:
    return cold::op_eq;
  case bc::opcode_t::op_ne:
    return cold::op_ne;
  case bc::opcode_t::op_and:
    return cold::op_and;
  case bc::opcode_t::op_or:
    return cold::op_or;
  case bc::opcode_t::op_const:
    return cold::op_const;
  case bc::opcode_t::op_string:
    return cold::op_string;
  case bc::opcode_t::op_drop:
    return cold::op_drop;
  case bc::opcode_t::op_dup:
    return cold::op_dup;
  case bc::opcode_t::op_swap:
    return cold::op_swap;
  case bc::opcode_t::op_ld_glo:
    return cold::op_ld_glo;
  case bc::opcode_t::op_ld_loc:
    return cold::op_ld_loc;
  case bc::opcode_t::op_ld_arg:
    return cold::op_ld_arg;
  case bc::opcode_t::op_st_glo:
    return cold::op_st_glo;
  case bc::opcode_t::op_st_loc:
    return cold::op_st_loc;
  case bc::opcode_t::op_st_arg:
    return cold::op_st_arg;
  case bc::opcode_t::op_jmp:
    return cold::op_jmp;
  case bc::opcode_t::op_cjmp_z:
    return cold::op_cjmp_z;
  case bc::opcode_t::op_cjmp_nz:
    return cold::op_cjmp_nz;
  case bc::opcode_t::op_call:
    return cold::op_call;
  case bc::opcode_t::op_begin:
    return cold::op_begin;
  case bc::opcode_t::op_end:
    return cold::op_end;
  case bc::opcode_t::op_line:
    return cold::op_line;
  case bc::opcode_t::op_eof:
    return cold::op_eof;
  case bc::opcode_t::op_sexp:
  case bc::opcode_t::op_sta:
  case bc::opcode_t::op_elem:
  case bc::opcode_t::op_ld_clo:
  case bc::opcode_t::op_st_clo:
  case bc::opcode_t::op_begin_closure:
  case bc::opcode_t::op_closure:
  case bc::opcode_t::op_callc:
  case bc::opcode_t::op_tag:
  case bc::opcode_t::op_array:
  case bc::opcode_t::op_fail:
  case bc::opcode_t::op_fail_keep:
  case bc::opcode_t::op_patt_str_cmp:
  case bc::opcode_t::op_patt_string:
  case bc::opcode_t::op_patt_array:
  case bc::opcode_t::op_patt_sexp:
  case bc::opcode_t::op_patt_boxed:
  case bc::opcode_t::op_patt_unboxed:
  case bc::opcode_t::op_patt_closure:
  case bc::opcode_t::op_barray:
    break;
  }

  fatalf("unexpected handler {:x}", std::to_underlying(handler_opcode));
}

} // namespace

void decode(std::span<op::word_t> text) {
  for (std::size_t i = 0; i < text.size(); ++i) {
    const auto handler_id = text[i].as_handler_id;
    text[i].as_handler_fn = decode(handler_id);

    const auto opcode = static_cast<bc::opcode_t>(handler_id);
    switch (opcode) {
    case bc::opcode_t::op_const:
    case bc::opcode_t::op_string:
    case bc::opcode_t::op_ld_glo:
    case bc::opcode_t::op_ld_loc:
    case bc::opcode_t::op_ld_arg:
    case bc::opcode_t::op_st_glo:
    case bc::opcode_t::op_st_loc:
    case bc::opcode_t::op_st_arg:
    case bc::opcode_t::op_jmp:
    case bc::opcode_t::op_cjmp_z:
    case bc::opcode_t::op_cjmp_nz:
    case bc::opcode_t::op_call:
    case bc::opcode_t::op_begin:
    case bc::opcode_t::op_line:
      ++i;
      break;
    case bc::opcode_t::op_add:
    case bc::opcode_t::op_sub:
    case bc::opcode_t::op_mul:
    case bc::opcode_t::op_div:
    case bc::opcode_t::op_mod:
    case bc::opcode_t::op_lt:
    case bc::opcode_t::op_le:
    case bc::opcode_t::op_gt:
    case bc::opcode_t::op_ge:
    case bc::opcode_t::op_eq:
    case bc::opcode_t::op_ne:
    case bc::opcode_t::op_and:
    case bc::opcode_t::op_or:
    case bc::opcode_t::op_sexp:
    case bc::opcode_t::op_sta:
    case bc::opcode_t::op_end:
    case bc::opcode_t::op_drop:
    case bc::opcode_t::op_dup:
    case bc::opcode_t::op_swap:
    case bc::opcode_t::op_eof:
      break;
    case bc::opcode_t::op_elem:
    case bc::opcode_t::op_ld_clo:
    case bc::opcode_t::op_st_clo:
    case bc::opcode_t::op_begin_closure:
    case bc::opcode_t::op_closure:
    case bc::opcode_t::op_callc:
    case bc::opcode_t::op_tag:
    case bc::opcode_t::op_array:
    case bc::opcode_t::op_fail:
    case bc::opcode_t::op_fail_keep:
    case bc::opcode_t::op_patt_str_cmp:
    case bc::opcode_t::op_patt_string:
    case bc::opcode_t::op_patt_array:
    case bc::opcode_t::op_patt_sexp:
    case bc::opcode_t::op_patt_boxed:
    case bc::opcode_t::op_patt_unboxed:
    case bc::opcode_t::op_patt_closure:
    case bc::opcode_t::op_barray:
      LAMAVM_UNEXPECTEDLY("unsupported opcode {}", opcode);
    }
  }
}

} // namespace lamavm::interpreter
