#pragma once

#include "interpreter/runtime/box.hpp"
#include "interpreter/runtime/state.hpp"

namespace lamavm::interpreter::cold {

void stack_push(box x);

box stack_pop();

box stack_pop_arg();

void stack_push_frame(frame f);

} // namespace lamavm::interpreter::cold
