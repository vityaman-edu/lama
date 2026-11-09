#include "arithmetic/sum.hpp"

#include <limits>

#include "gtest/gtest.h"

namespace {

TEST(arithmetic, zero) {
  EXPECT_EQ(sum(0, 0), 0);
  EXPECT_EQ(sum(42, 0), 42);
  EXPECT_EQ(sum(0, -42), -42);
}

TEST(arithmetic, positive) {
  EXPECT_EQ(sum(20, 22), 42);
}

TEST(arithmetic, negative) {
  EXPECT_EQ(sum(-20, -22), -42);
}

TEST(arithmetic, mixed) {
  EXPECT_EQ(sum(-20, 22), 2);
  EXPECT_EQ(sum(20, -22), -2);
  EXPECT_EQ(sum(42, -42), 0);
}

TEST(arithmetic, limits) {
  constexpr int min = std::numeric_limits<int>::min();
  constexpr int max = std::numeric_limits<int>::max();

  EXPECT_EQ(sum(min, 0), min);
  EXPECT_EQ(sum(max, 0), max);
  EXPECT_EQ(sum(min + 1, -1), min);
  EXPECT_EQ(sum(max - 1, 1), max);
  EXPECT_EQ(sum(min, max), -1);
}

} // namespace
