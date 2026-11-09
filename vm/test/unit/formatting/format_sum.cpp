#include "formatting/format_sum.hpp"

#include <limits>
#include <string>

#include "gtest/gtest.h"

namespace {

TEST(formatting, zero) {
  EXPECT_EQ(format_sum(42, -42), "0");
}

TEST(formatting, signs) {
  EXPECT_EQ(format_sum(20, 22), "42");
  EXPECT_EQ(format_sum(-20, -22), "-42");
}

TEST(formatting, limits) {
  constexpr int min = std::numeric_limits<int>::min();
  constexpr int max = std::numeric_limits<int>::max();

  EXPECT_EQ(format_sum(min, 0), std::to_string(min));
  EXPECT_EQ(format_sum(max, 0), std::to_string(max));
}

} // namespace
