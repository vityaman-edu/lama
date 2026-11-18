#pragma once

#include "interpreter/runtime/box.hpp"
#include "interpreter/runtime/state.hpp"

#include <cstdint>
#include <span>

namespace lamavm::interpreter::cold {

const char* c_str(std::int32_t offset);

box& stack_push();

void stack_push(box x);

box stack_pop();

void stack_pop(std::int64_t count);

std::span<box> stack_args(std::int64_t count);

box stack_pop_arg();

void stack_push_frame(frame f);

} // namespace lamavm::interpreter::cold
