#include "formatting/format_sum.hpp"

#include "arithmetic/sum.hpp"

#include <string>

auto format_sum(int a, int b) -> std::string {
  return std::to_string(sum(a, b));
}
