#include "stack.hpp"

#include "bc/limit.hpp"
#include "box.hpp"
#include "core/assert.hpp"
#include "state.hpp"

#include "os/mmap.hpp"
#include "os/page.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <span>

#include <sys/mman.h>

namespace lamavm::interpreter {

namespace {

std::size_t guard_size = 0;

} // namespace

std::span<box> stack;

os::mmap map_stack() {
  using storage = std::array<box, max_stack_size / sizeof(box)>;
  static_assert(sizeof(storage) == max_stack_size);

  const auto page_size = os::page_size();
  LAMAVM_EXPECT_B(max_stack_size % page_size, ==, 0U);

  os::mmap mstack(page_size + max_stack_size + page_size, PROT_NONE);

  const auto memory = mstack.pages(1, mstack.pages() - 1);
  os::mprotect(memory, PROT_READ | PROT_WRITE);

  guard_size = page_size;
  stack = *reinterpret_cast<storage*>(memory.data());
  sp = stack.data() + stack.size();

  return mstack;
}

bool is_top_stack_guard(std::uintptr_t address) noexcept {
  const auto start = reinterpret_cast<std::uintptr_t>(stack.data()) - guard_size;
  return !stack.empty() && address >= start && address - start < guard_size;
}

bool is_bot_stack_guard(std::uintptr_t address) noexcept {
  const auto start = reinterpret_cast<std::uintptr_t>(stack.data()) + stack.size_bytes();
  return !stack.empty() && address >= start && address - start < guard_size;
}

} // namespace lamavm::interpreter
