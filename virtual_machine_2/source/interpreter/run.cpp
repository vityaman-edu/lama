#include "run.hpp"

#include "bc/units.hpp"
#include "core/assert.hpp"

#include "decode.hpp"

#include "interpreter/cold/handler.hpp"
#include "interpreter/runtime/box.hpp"
#include "interpreter/runtime/gc.hpp"
#include "interpreter/runtime/state.hpp"

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <utility>
#include <vector>

namespace lamavm::interpreter {

namespace {

void exec() {
  fp = sp;
  pc = up->text.data();

  LAMAVM_ASSERT_B(0U, <, up->text.size());
  LAMAVM_ASSERT(up->text.back().as_handler_fn == cold::op_eof);

  stack_push(box(std::int64_t{0}));
  stack_push(box(std::int64_t{0}));

  stack_push_frame({
    .n_args = 2,
    .fp = fp,
    .pc = up->text.data() + up->text.size() - 1,
    .sp = sp,
    .up = up,
  });

  LAMAVM_DISPATCH_JUMP();
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

  constexpr size_t stack_size = size_t{1024} * 1024 / sizeof(box);
  stack = std::vector<box>(stack_size);

  sp = stack.data() + stack.size();

  gc::init();
  gc::set_bot(sp);

  for (std::size_t i = 0; i < globals; ++i) {
    stack_push(box(std::int64_t{0}));
  }

  for (const uctx& u : units) {
    up = &u;
    exec();
  }

  std::exit(0); // NOLINT(concurrency-mt-unsafe)
}

} // namespace lamavm::interpreter
