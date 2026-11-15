#pragma once

#include "box.hpp"

#include "bc/units.hpp"

#include "op/word.hpp"

#include <span>
#include <vector>

namespace lamavm::interpreter {

struct uctx {
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

} // namespace lamavm::interpreter
