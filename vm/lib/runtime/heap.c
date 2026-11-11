#include <gc.h>

#include <stddef.h>

extern size_t __gc_stack_top, __gc_stack_bottom;

void lamavm_heap_init(aint *stack, size_t size) {
  __init();
  __gc_stack_top = (size_t)(stack + size - 1);
  __gc_stack_bottom = (size_t)(stack + size);
}

void lamavm_heap_stack_top(aint *top) {
  __gc_stack_top = (size_t)top;
}
