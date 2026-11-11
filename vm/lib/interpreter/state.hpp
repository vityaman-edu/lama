#pragma once

#include <cstddef>
#include <span>
#include <string>
#include <string_view>

namespace lamavm::interpreter {

struct state {
private:
  std::string area_string_table;

public:
  state(std::string string_table, std::span<const std::byte> code);

  state(const state &) = delete;
  state(state &&) = delete;
  auto operator=(const state &) -> state & = delete;
  auto operator=(state &&) -> state & = delete;
  ~state();

  // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
  std::string_view string_table;

  // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
  std::span<const std::byte> code;

  // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
  std::size_t ip = 0;

  // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
  std::size_t bp;
};

} // namespace lamavm::interpreter
