#include "input.hpp"

#include "listeners.hpp"

#include "bc/code_reader.hpp"
#include "bc/file_reader.hpp"

#include <gtest/gtest.h>

#include <fuzztest/fuzztest.h>

#include <cstddef>
#include <span>
#include <sstream>
#include <string>

namespace lamavm::bc::test {

void read_random_bytes(const std::string &bytes) {
  EXPECT_NO_THROW({
    std::istringstream input(bytes);
    noop_file_listener listener;
    const auto result = read_file(input, listener);
    (void)result;
  });
}

void read_random_code(const std::string &bytes) {
  EXPECT_NO_THROW({
    noop_code_listener listener;
    const auto result = read_code(std::as_bytes(std::span(bytes)), listener);
    (void)result;
  });
}

FUZZ_TEST(BytecodeInput, read_random_bytes).WithDomains(fuzztest::Arbitrary<std::string>());
FUZZ_TEST(BytecodeInput, read_random_code).WithDomains(fuzztest::Arbitrary<std::string>());

} // namespace lamavm::bc::test
