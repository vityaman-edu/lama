#pragma once

#include "box.hpp"
#include "gc.hpp"

#include "bc/units.hpp"

#include "core/compiler.hpp"

#include "op/word.hpp"

#include <cstdint>
#include <span>
#include <vector>

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define LAMAVM_DISPATCH_JUMP() LAMAVM_MUSTTAIL return pc->as_handler_fn()

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define LAMAVM_DISPATCH()                                                                          \
  ++pc;                                                                                            \
  LAMAVM_DISPATCH_JUMP()

namespace lamavm::interpreter {

struct uctx {
  const char* name = nullptr;
  std::span<const char> data;
  std::span<const op::word_t> text;
  std::size_t glo_base = 0;
};

extern bc::symbol::index symbols;
extern std::vector<uctx> units;
extern std::vector<box> stack;

extern const op::word_t* pc;
extern box* sp;
extern box* fp;
extern const uctx* up;

struct frame {
  static constexpr std::int64_t size = 5;

  std::int64_t n_args = 0;
  box* fp = nullptr;
  const op::word_t* pc = nullptr;
  box* sp = nullptr;
  const uctx* up = nullptr;
};

void stack_push(box x);

box stack_pop();

void stack_pop_frame();

std::int64_t stack_n_args();

auto gc_call(auto f) {
  gc::set_top(sp - 1);
  return f();
}

} // namespace lamavm::interpreter
