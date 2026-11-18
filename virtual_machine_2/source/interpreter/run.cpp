#include "run.hpp"

#include "bc/units.hpp"
#include "core/assert.hpp"

#include "decode.hpp"

#include "interpreter/cold/handler.hpp"
#include "interpreter/cold/state.hpp"
#include "interpreter/runtime/box.hpp"
#include "interpreter/runtime/gc.hpp"
#include "interpreter/runtime/state.hpp"
#include "interpreter/warm/handler.hpp"
#include "op/word.hpp"

#include <array>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <utility>
#include <vector>

namespace lamavm::interpreter {

namespace {

void exec() {
  fp = nullptr;
  pc = up->text.data();

  LAMAVM_ASSERT_B(0U, <, up->text.size());
  LAMAVM_ASSERT(up->text.back().as_handler_fn == cold::op_eof);

  cold::stack_push(box(std::int64_t{0}));
  cold::stack_push(box(std::int64_t{0}));

  std::array<op::word_t, 3> startup{{
    {.as_target = nullptr},
    {.as_call = {.unit_index = 0, .count = 2}},
    {.as_handler_fn = cold::op_eof},
  }};
  const auto entry = pc->as_handler_fn;
  if (entry == warm::op_begin || entry == warm::op_begin_no_locals) {
    LAMAVM_EXPECT_B(pc[1].as_i32.arg0, ==, startup[1].as_call.count);
    startup[0].as_target = pc;
  }

  cold::stack_push_frame({
    .fp = fp,
    .pc = &startup.back(),
    .sp = sp,
    .up = up,
  });

  pc->as_handler_fn();
}

} // namespace

void run(bc::units units1) {
  static std::atomic_bool is_loaded = false;
  LAMAVM_EXPECT(!is_loaded.exchange(true));

  std::size_t globals = 0;

  symbols = std::move(units1.symbols);

  units.reserve(units1.initialization_order.size());
  for (const auto& it : units1.initialization_order) {
    auto& [name, unit] = *it;

    decode(unit.code_section);

    units.push_back({
      .name = name.c_str(),
      .data = unit.string_table,
      .text = unit.code_section,
      .glo_base = globals,
    });

    globals += unit.globals_count;
  }

  cold::globals_count = globals;

  constexpr size_t stack_size = size_t{1024} * 1024 / sizeof(box);
  stack = std::vector<box>(stack_size);

  sp = stack.data() + stack.size();

  gc::init();
  gc::set_bot(sp);

  for (std::size_t i = 0; i < globals; ++i) {
    cold::stack_push(box(std::int64_t{0}));
  }

  for (const uctx& u : units) {
    up = &u;
    exec();
  }

  std::exit(0); // NOLINT(concurrency-mt-unsafe)
}

} // namespace lamavm::interpreter
