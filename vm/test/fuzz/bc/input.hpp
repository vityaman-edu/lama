#pragma once

#include "bc/input.hpp"
#include "bc/listener.hpp"

#include <gtest/gtest.h>

#include <fuzztest/fuzztest.h>

#include <sstream>
#include <string>

namespace lamavm::bc::test {

inline void read_random_bytes(const std::string &bytes) {
  EXPECT_NO_THROW({
    std::istringstream input(bytes);
    base_listener listener;
    const auto result = read_file(input, listener);
    (void)result;
  });
}

FUZZ_TEST(BytecodeInput, read_random_bytes).WithDomains(fuzztest::Arbitrary<std::string>());

} // namespace lamavm::bc::test
