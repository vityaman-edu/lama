#pragma once

#include "core/exception.hpp"

#include <cstddef>
#include <cstdint>
#include <expected>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace lamavm::interpreter {

struct state {
private:
  std::string area_string_table;
  std::vector<std::int64_t> area_stack;

public:
  state(std::string string_table, std::span<const std::byte> code, std::vector<std::int64_t> stack);

  state(const state &) = delete;
  state(state &&) = delete;
  auto operator=(const state &) -> state & = delete;
  auto operator=(state &&) -> state & = delete;
  ~state() = default;

  auto stack_push(std::int64_t value) -> status;

  auto stack_pop() -> std::expected<std::int64_t, error>;

  [[nodiscard]] auto stack_top() const -> std::expected<std::int64_t, error>;

  // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
  std::string_view string_table;

  // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
  std::span<const std::byte> code;

  // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
  std::span<std::int64_t> stack;

  // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
  std::size_t ip = 0;

  // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
  std::size_t sp = stack.size();

  // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
  std::size_t bp = sp;
};

} // namespace lamavm::interpreter
