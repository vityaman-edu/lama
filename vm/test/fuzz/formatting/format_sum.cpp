#include "formatting/format_sum.hpp"

#include <cstdint>
#include <limits>
#include <string>
#include <utility>

#include "fuzztest/fuzztest.h"
#include "gtest/gtest.h"

namespace {

auto sum_fits_in_int(const std::pair<int, int> &operands) -> bool {
  const auto expected =
      static_cast<std::int64_t>(operands.first) + operands.second;
  return expected >= std::numeric_limits<int>::min() &&
         expected <= std::numeric_limits<int>::max();
}

void format_sum_matches_decimal_addition(const std::pair<int, int> &operands) {
  const auto [lhs, rhs] = operands;
  const auto expected = static_cast<std::int64_t>(lhs) + rhs;

  EXPECT_EQ(format_sum(lhs, rhs), std::to_string(expected));
}

// NOLINTNEXTLINE(cert-err58-cpp,cppcoreguidelines-avoid-non-const-global-variables)
FUZZ_TEST(formatting, format_sum_matches_decimal_addition)
    .WithDomains(fuzztest::Filter(sum_fits_in_int,
                                  fuzztest::Arbitrary<std::pair<int, int>>()));

} // namespace
