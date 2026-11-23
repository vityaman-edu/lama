#include "stack_fault.hpp"

#include "core/arena.hpp"
#include "core/emergency.hpp"
#include "op/word.hpp"
#include "os/signal.hpp"
#include "stack.hpp"
#include "state.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <span>
#include <string_view>

namespace lamavm::interpreter {

namespace {

constexpr std::size_t max_unit_name_size = 64;
constexpr std::size_t label_size =
  std::string_view("VM stack underflow at 0x, unit \"\", op ").size();
constexpr std::size_t max_address_size = std::numeric_limits<std::uintptr_t>::digits / 4;
constexpr std::size_t max_op_index_size = std::numeric_limits<std::ptrdiff_t>::digits10 + 1;
constexpr auto max_message_size =
  label_size + max_unit_name_size + max_address_size + max_op_index_size;

constinit thread_local std::array<char, max_message_size> fault_message{};
std::span<char> fault_arena;

std::string_view unit_name(const uctx* unit) noexcept {
  if (unit == nullptr || unit->name == nullptr) {
    return "<unknown>";
  }

  const char* name = unit->name;
  std::size_t size = 0;
  while (size < max_unit_name_size && name[size] != '\0') {
    ++size;
  }

  return {name, size};
}

void append_op_index(arena& message, const uctx* unit, const op::word_t* ip) noexcept {
  if (unit != nullptr && ip != nullptr) {
    const auto base = reinterpret_cast<std::uintptr_t>(unit->text.data());
    const auto address = reinterpret_cast<std::uintptr_t>(ip);
    if (address >= base && address - base < unit->text.size_bytes()) {
      message.append_decimal(static_cast<std::ptrdiff_t>((address - base) / sizeof(op::word_t)));
      return;
    }
  }

  message.append("<unknown>");
}

} // namespace

void register_stack_fault_handler() {
  fault_arena = fault_message;
  os::register_page_fault_handler(stack_fault_handler);
}

void stack_fault_handler(os::page_fault fault) noexcept {
  const bool is_overflow = is_top_stack_guard(fault.address);
  const bool is_underflow = is_bot_stack_guard(fault.address);
  if (!is_overflow && !is_underflow) {
    return;
  }

  arena message(fault_arena);
  message.append(is_overflow ? "VM stack overflow at 0x" : "VM stack underflow at 0x");
  message.append_hex(fault.address);
  message.append(", unit \"");
  message.append(unit_name(up));
  message.append("\", op ");
  append_op_index(message, up, pc);
  signal_emergency(message.view());
}

} // namespace lamavm::interpreter
