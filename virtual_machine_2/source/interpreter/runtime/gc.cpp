#include "gc.hpp"

extern "C" {
#include <contrib/runtime/gc.h>
}

#include <cstddef>

extern "C" {
extern std::size_t __gc_stack_top;    // NOLINT
extern std::size_t __gc_stack_bottom; // NOLINT
}

namespace lamavm::interpreter {

void gc::init() {
  __gc_init();
}

std::size_t& gc::bot() {
  return __gc_stack_bottom;
}

std::size_t& gc::top() {
  return __gc_stack_top;
}

} // namespace lamavm::interpreter
