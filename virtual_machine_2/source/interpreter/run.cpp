#include "run.hpp"

#include "bc/layout.hpp"
#include "bc/units.hpp"
#include "core/assert.hpp"
#include "core/log.hpp"

#include "interpreter/cold/handler.hpp"
#include "interpreter/runtime/box.hpp"
#include "interpreter/runtime/gc.hpp"
#include "interpreter/runtime/state.hpp"

#include "op/handler.hpp"

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <limits>
#include <type_traits>
#include <utility>
#include <vector>

namespace lamavm::interpreter {

namespace {

void step(bool& is_running) {
  const auto offset = pc - up->text.data();
  const auto handler_id = (*pc++).as_handler_id;

  const auto max = std::numeric_limits<std::underlying_type_t<bc::opcode_t>>::max();
  if (max < handler_id) {
    fatalf("unexpected handler overflow {:x}", handler_id);
  }

  const auto handler_opcode = static_cast<bc::opcode_t>(handler_id);
  tracef("step {} pc={}", handler_opcode, offset);

  switch (handler_opcode) {
  case bc::opcode_t::op_add:
    cold::op_add();
    return;
  case bc::opcode_t::op_sub:
    cold::op_sub();
    return;
  case bc::opcode_t::op_mul:
    cold::op_mul();
    return;
  case bc::opcode_t::op_div:
    cold::op_div();
    return;
  case bc::opcode_t::op_mod:
    cold::op_mod();
    return;
  case bc::opcode_t::op_lt:
    cold::op_lt();
    return;
  case bc::opcode_t::op_le:
    cold::op_le();
    return;
  case bc::opcode_t::op_gt:
    cold::op_gt();
    return;
  case bc::opcode_t::op_ge:
    cold::op_ge();
    return;
  case bc::opcode_t::op_eq:
    cold::op_eq();
    return;
  case bc::opcode_t::op_ne:
    cold::op_ne();
    return;
  case bc::opcode_t::op_and:
    cold::op_and();
    return;
  case bc::opcode_t::op_or:
    cold::op_or();
    return;
  case bc::opcode_t::op_const:
    cold::op_const();
    return;
  case bc::opcode_t::op_string:
    cold::op_string();
    return;
  case bc::opcode_t::op_drop:
    cold::op_drop();
    return;
  case bc::opcode_t::op_dup:
    cold::op_dup();
    return;
  case bc::opcode_t::op_swap:
    cold::op_swap();
    return;
  case bc::opcode_t::op_ld_glo:
    cold::op_ld_glo();
    return;
  case bc::opcode_t::op_ld_loc:
    cold::op_ld_loc();
    return;
  case bc::opcode_t::op_ld_arg:
    cold::op_ld_arg();
    return;
  case bc::opcode_t::op_st_glo:
    cold::op_st_glo();
    return;
  case bc::opcode_t::op_st_loc:
    cold::op_st_loc();
    return;
  case bc::opcode_t::op_st_arg:
    cold::op_st_arg();
    return;
  case bc::opcode_t::op_jmp:
    cold::op_jmp();
    return;
  case bc::opcode_t::op_cjmp_z:
    cold::op_cjmp_z();
    return;
  case bc::opcode_t::op_cjmp_nz:
    cold::op_cjmp_nz();
    return;
  case bc::opcode_t::op_call:
    cold::op_call();
    return;
  case bc::opcode_t::op_begin:
    cold::op_begin();
    return;
  case bc::opcode_t::op_end:
    cold::op_end();
    return;
  case bc::opcode_t::op_line:
    cold::op_line();
    return;
  case bc::opcode_t::op_eof:
    is_running = false;
    return;
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

void exec(const uctx* up1) {
  fp = sp;
  pc = up1->text.data();
  up = up1;

  constexpr auto eof_id = static_cast<op::handler_id_t>(bc::opcode_t::op_eof);
  LAMAVM_ASSERT_B(0U, <, up->text.size());
  LAMAVM_ASSERT_B(up->text.back().as_handler_id, ==, eof_id);

  stack_push(box(std::int64_t{0}));
  stack_push(box(std::int64_t{0}));

  stack_push_frame({
    .n_args = 2,
    .fp = fp,
    .pc = up->text.data() + up->text.size() - 1,
    .sp = sp,
    .up = up,
  });

  for (bool is_running = true; is_running;) {
    step(is_running);
  }
}

} // namespace

void run(bc::units units1) {
  static std::atomic_bool is_loaded = false;
  LAMAVM_EXPECT(!is_loaded.exchange(true));

  std::size_t globals = 0;

  symbols = std::move(units1.symbols);

  units.reserve(units1.initialization_order.size());
  for (const auto& it : units1.initialization_order) {
    const auto& [name, unit] = *it;

    units.push_back({
      .data = unit.string_table,
      .text = unit.code_section,
      .glo_base = globals,
    });

    globals += unit.globals_count;
  }

  constexpr size_t stack_size = size_t{1024} * 1024 / sizeof(box);
  stack = std::vector<box>(stack_size);

  sp = stack.data() + stack.size();

  gc::init();
  gc::set_bot(sp);

  for (std::size_t i = 0; i < globals; ++i) {
    stack_push(box(std::int64_t{0}));
  }

  for (const uctx& u : units) {
    exec(&u);
  }

  std::exit(0); // NOLINT(concurrency-mt-unsafe)
}

} // namespace lamavm::interpreter
