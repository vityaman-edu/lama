#pragma once

#include "box.hpp"
#include "gc.hpp"

#include "bc/units.hpp"

#include "core/compiler.hpp"

#include "op/word.hpp"

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

const char* c_str(std::int32_t offset);

auto gc_call(auto f) {
  gc::set_top(sp - 1);
  return f();
}

} // namespace lamavm::interpreter
