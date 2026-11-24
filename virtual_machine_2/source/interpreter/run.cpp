#include "run.hpp"

#include "bc/units.hpp"
#include "core/assert.hpp"

#include "decode.hpp"

#include "interpreter/cold/handler.hpp"
#include "interpreter/cold/state.hpp"
#include "interpreter/runtime/box.hpp"
#include "interpreter/runtime/gc.hpp"
#include "interpreter/runtime/stack.hpp"
#include "interpreter/runtime/stack_fault.hpp"
#include "interpreter/runtime/state.hpp"
#include "op/word.hpp"
#include "os/mmap.hpp"

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <utility>

namespace lamavm::interpreter {

namespace {

void exec() {
  fp = nullptr;
  pc = up->text.data();

  LAMAVM_ASSERT_B(0U, <, up->text.size());

  auto* caller_sp = static_cast<box*>(sp);
  cold::stack_push(box(std::int64_t{0}));
  cold::stack_push(box(std::int64_t{0}));

  op::word_t startup_return{.as_handler_fn = cold::op_eof};
  cold::stack_push_frame({
    .n_args = 2,
    .fp = fp,
    .pc = &startup_return,
    .sp = caller_sp,
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

  const os::mmap mstack = map_stack();
  register_stack_fault_handler();

  gc::init();

  const auto* stack_bot = static_cast<box*>(sp);
  gc::bot() = reinterpret_cast<std::size_t>(stack_bot);

  for (std::size_t i = 0; i < globals; ++i) {
    cold::stack_push(box(std::int64_t{0}));
  }

  for (const uctx& u : units) {
    up = &u;
    exec();
  }

  stack = {};
  sp = nullptr;
  gc::bot() = 0;
  fp = nullptr;
}

} // namespace lamavm::interpreter
