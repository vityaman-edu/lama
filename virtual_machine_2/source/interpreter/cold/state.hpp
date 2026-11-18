#pragma once

#include "interpreter/runtime/box.hpp"

#include <cstdint>

namespace lamavm::op {
union word_t;
} // namespace lamavm::op

namespace lamavm::interpreter {

struct uctx;

namespace cold {

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

box stack_pop_arg();

void stack_push_frame(frame f);

void stack_pop_frame();

std::int64_t stack_n_args();

} // namespace cold

} // namespace lamavm::interpreter
