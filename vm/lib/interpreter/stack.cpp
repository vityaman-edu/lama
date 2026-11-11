#include "stack.hpp"

#include "core/exception.hpp"
#include "runtime/heap.hpp"

#include <array>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <span>

namespace lamavm::interpreter {
namespace {

constexpr std::size_t stack_capacity = (std::size_t{1024} * 1024) / sizeof(std::int64_t);

struct stack_data {
  std::array<std::int64_t, stack_capacity + 1> words{};
  std::size_t sp = stack_capacity;
  std::atomic_flag is_in_use;
};

auto data() -> stack_data & {
  static stack_data instance;
  return instance;
}

} // namespace

void stack::acquire() {
  auto &area = data();
  if (area.is_in_use.test_and_set(std::memory_order_acquire)) {
    throw exception("an interpreter is already active");
  }

  area.sp = stack_capacity;
  runtime::heap_init(area.words.data(), area.words.size());
  sync();
}

void stack::release() {
  runtime::heap_shutdown();
  data().sp = stack_capacity;
  data().is_in_use.clear(std::memory_order_release);
}

void stack::sync() {
  auto &area = data();
  runtime::heap_stack_top(std::span(area.words).subspan(area.sp).data());
}

auto stack::push(std::int64_t value) -> status {
  auto &area = data();
  if (area.sp == 0) {
    return std::unexpected("stack overflow");
  }

  --area.sp;
  values().front() = value;
  sync();
  return {};
}

auto stack::pop() -> std::expected<std::int64_t, error> {
  if (size() == 0) {
    return std::unexpected("stack underflow");
  }

  const auto value = values().front();
  ++data().sp;
  sync();
  return value;
}

auto stack::drop(std::size_t count) -> status {
  if (count > size()) {
    return std::unexpected("stack underflow");
  }

  data().sp += count;
  sync();
  return {};
}

auto stack::top() -> std::expected<std::int64_t, error> {
  if (size() == 0) {
    return std::unexpected("stack underflow");
  }

  return values().front();
}

auto stack::values() -> std::span<std::int64_t> {
  auto &area = data();
  return std::span(area.words).subspan(area.sp + 1);
}

auto stack::size() -> std::size_t {
  return stack_capacity - data().sp;
}

auto stack::capacity() -> std::size_t {
  return stack_capacity;
}

} // namespace lamavm::interpreter
