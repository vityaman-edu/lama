#pragma once

#include "interpreter/runtime/box.hpp"
#include "interpreter/runtime/state.hpp"

#include <cstdint>

namespace lamavm::interpreter::cold {

const char* c_str(std::int32_t offset);

void stack_push(box x);

box stack_pop();

box stack_pop_arg();

void stack_push_frame(frame f);

} // namespace lamavm::interpreter::cold
