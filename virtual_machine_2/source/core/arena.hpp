#pragma once

#include <cstddef>
#include <cstdint>
#include <span>
#include <string_view>

namespace lamavm {

class arena final {
public:
  explicit arena(std::span<char> buffer) noexcept;

  void append(std::string_view text) noexcept;
  void append_decimal(std::ptrdiff_t value) noexcept;
  void append_hex(std::uintptr_t value) noexcept;

  [[nodiscard]] std::string_view view() const noexcept;

private:
  std::span<char> buffer;
  char* next;
};

} // namespace lamavm
