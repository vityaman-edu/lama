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

void gc::set_bot(void* x) {
  __gc_stack_bottom = reinterpret_cast<std::size_t>(x);
}

void gc::set_top(void* x) {
  __gc_stack_top = reinterpret_cast<std::size_t>(x);
}

} // namespace lamavm::interpreter
