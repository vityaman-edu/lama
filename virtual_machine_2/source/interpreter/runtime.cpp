#include "runtime.hpp"

extern "C" {
#include <contrib/runtime/gc.h>
#include <contrib/runtime/runtime_common.h>
}

#include <cstdint>

extern "C" {
extern size_t __gc_stack_top;    // NOLINT
extern size_t __gc_stack_bottom; // NOLINT
}

namespace lamavm::interpreter {

bool is_unboxed(std::int64_t x) {
  return UNBOXED(x);
}

std::int64_t box(std::int64_t x) {
  return BOX(x);
}

std::int64_t unbox(std::int64_t x) {
  return UNBOX(x);
}

void gc_init() {
  __gc_init();
}

void gc_set_stack_bottom(void* x) {
  __gc_stack_bottom = reinterpret_cast<size_t>(x);
}

void gc_set_stack_top(void* x) {
  __gc_stack_top = reinterpret_cast<size_t>(x);
}

} // namespace lamavm::interpreter
