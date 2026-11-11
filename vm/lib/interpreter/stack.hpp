#pragma once

#include "core/exception.hpp"

#include <cstddef>
#include <cstdint>
#include <expected>
#include <span>

namespace lamavm::interpreter {

class stack {
  friend struct state;

public:
  stack() = delete;

  static auto push(std::int64_t value) -> status;
  static auto pop() -> std::expected<std::int64_t, error>;
  static auto drop(std::size_t count) -> status;
  [[nodiscard]] static auto top() -> std::expected<std::int64_t, error>;
  [[nodiscard]] static auto values() -> std::span<std::int64_t>;
  [[nodiscard]] static auto size() -> std::size_t;
  [[nodiscard]] static auto capacity() -> std::size_t;

private:
  static void acquire();
  static void release();
  static void sync();
};

} // namespace lamavm::interpreter
