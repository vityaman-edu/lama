#pragma once

#include <cstddef>
#include <cstdint>

namespace lamavm::runtime {

auto heap_init(std::int64_t *stack, std::size_t size)
  -> void __asm__(LAMAVM_RUNTIME_PREFIX "lamavm_heap_init");

auto heap_stack_top(std::int64_t *top)
  -> void __asm__(LAMAVM_RUNTIME_PREFIX "lamavm_heap_stack_top");

auto heap_shutdown() -> void __asm__(LAMAVM_RUNTIME_PREFIX "__shutdown");

} // namespace lamavm::runtime
