#pragma once

#include <cstddef>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace lamavm::interpreter {

struct state {
private:
  std::string area_string_table;
  std::vector<std::byte> area_stack;

public:
  state(std::string string_table, std::span<const std::byte> code, std::vector<std::byte> stack);

  state(const state &) = delete;
  state(state &&) = delete;
  auto operator=(const state &) -> state & = delete;
  auto operator=(state &&) -> state & = delete;
  ~state() = default;

  // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
  std::string_view string_table;

  // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
  std::span<const std::byte> code;

  // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
  std::span<std::byte> stack;

  // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
  std::size_t ip = 0;

  // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
  std::size_t sp = stack.size();

  // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
  std::size_t bp = sp;
};

} // namespace lamavm::interpreter
