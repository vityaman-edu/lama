#include "formatting/format_sum.hpp"

#include <iostream>

auto main() -> int {
  constexpr int first = 20;
  constexpr int second = 22;

  std::cout << format_sum(first, second) << '\n';
}
