#include "arena.hpp"

#include <algorithm>
#include <charconv>
#include <cstddef>
#include <cstdint>
#include <span>
#include <string_view>
#include <system_error>

namespace lamavm {

arena::arena(std::span<char> buffer) noexcept
  : buffer(buffer)
  , next(buffer.data()) {
}

void arena::append(std::string_view text) noexcept {
  const auto available = static_cast<std::size_t>(buffer.data() + buffer.size() - next);
  text.remove_suffix(text.size() - std::min(text.size(), available));
  next = std::ranges::copy(text, next).out;
}

void arena::append_decimal(std::ptrdiff_t value) noexcept {
  const auto [end, error] = std::to_chars(next, buffer.data() + buffer.size(), value);
  if (error == std::errc{}) {
    next = end;
  }
}

void arena::append_hex(std::uintptr_t value) noexcept {
  constexpr int base = 16;
  const auto [end, error] = std::to_chars(next, buffer.data() + buffer.size(), value, base);
  if (error == std::errc{}) {
    next = end;
  }
}

std::string_view arena::view() const noexcept {
  return {buffer.data(), static_cast<std::size_t>(next - buffer.data())};
}

} // namespace lamavm
