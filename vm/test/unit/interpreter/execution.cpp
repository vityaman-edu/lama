#include "bc/layout.hpp"
#include "core/exception.hpp"
#include "interpreter/loop.hpp"
#include "runtime/box.hpp"

#include <gtest/gtest.h>

#include <array>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <limits>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace lamavm::interpreter {
namespace {

template <typename Op>
void append(std::vector<std::byte> &code, const Op &op) {
  const auto bytes = std::as_bytes(std::span(&op, 1));
  code.insert(code.end(), bytes.begin(), bytes.end());
}

TEST(RuntimeBox, EncodesAndDecodesSigned63BitIntegers) {
  struct example {
    std::int64_t number;
    std::int64_t tagged;
  };

  constexpr auto limit = static_cast<std::int64_t>(std::uint64_t{1} << 62U);
  constexpr std::array examples{
    example{.number = -limit, .tagged = std::numeric_limits<std::int64_t>::min() + 1},
    example{.number = -42, .tagged = -83},
    example{.number = -1, .tagged = -1},
    example{.number = 0, .tagged = 1},
    example{.number = 1, .tagged = 3},
    example{.number = 42, .tagged = 85},
    example{.number = limit - 1, .tagged = std::numeric_limits<std::int64_t>::max()},
  };

  for (const auto &[number, tagged] : examples) {
    EXPECT_EQ(runtime::box(number), tagged);
    EXPECT_EQ(runtime::unbox(tagged), number);
  }
}

TEST(InterpreterExecution, Computes42And43BeforeUnsupportedEnd) {
  constexpr std::int32_t initial = 42;
  for (const auto is_addition : {false, true}) {
    std::vector<std::byte> code;
    append(code, bc::begin{});
    append(code, bc::constant{.value = initial});

    if (is_addition) {
      append(code, bc::constant{.value = 1});
      append(code, bc::add{});
    }

    append(code, bc::end{});
    append(code, bc::eof{});
    loop machine({}, code);
    EXPECT_EQ(machine.run(), std::unexpected(error("unsupported instruction: end")));
    EXPECT_EQ(machine.stack_top(), (2 * (initial + static_cast<int>(is_addition))) + 1);
  }
}

TEST(InterpreterExecution, TagsSignedConstants) {
  constexpr std::array values{
    std::numeric_limits<std::int32_t>::min(),
    std::int32_t{-42},
    std::int32_t{0},
    std::int32_t{42},
    std::numeric_limits<std::int32_t>::max(),
  };
  loop machine({}, {});
  for (const auto value : values) {
    ASSERT_TRUE(machine(bc::constant{.value = value}));
    EXPECT_EQ(machine.stack_top(), (static_cast<std::int64_t>(value) * 2) + 1);
  }
}

TEST(InterpreterExecution, PushesPointersIntoStringTable) {
  std::vector<std::byte> code;
  append(code, bc::begin{});
  append(code, bc::string{.string_offset = 4});
  append(code, bc::end{});
  loop machine(std::string("Std\0abacaba", sizeof("Std\0abacaba")), code);
  EXPECT_EQ(machine.run(), std::unexpected(error("unsupported instruction: end")));
  const auto top = machine.stack_top();
  ASSERT_TRUE(top);
  const std::string_view value(std::bit_cast<const char *>(*top));
  EXPECT_EQ(value, "abacaba");

  ASSERT_TRUE(machine(bc::string{.string_offset = 5}));
  EXPECT_EQ(machine.stack_top(), std::bit_cast<std::int64_t>(value.substr(1).data()));

  ASSERT_TRUE(machine(bc::string{.string_offset = 11}));
  EXPECT_EQ(machine.stack_top(), std::bit_cast<std::int64_t>(value.substr(value.size()).data()));
}

TEST(InterpreterExecution, CallsEveryRuntimeBinopWithLeftThenRight) {
  struct example {
    bc::opcode opcode;
    std::int32_t left;
    std::int32_t right;
    std::int64_t expected;
  };

  constexpr std::array examples{
    example{.opcode = bc::opcode::add, .left = 42, .right = 1, .expected = 43},
    example{.opcode = bc::opcode::sub, .left = 1, .right = 42, .expected = -41},
    example{.opcode = bc::opcode::mul, .left = -6, .right = 7, .expected = -42},
    example{.opcode = bc::opcode::div, .left = -43, .right = 6, .expected = -7},
    example{.opcode = bc::opcode::mod, .left = -43, .right = 6, .expected = -1},
    example{.opcode = bc::opcode::lt, .left = 1, .right = 2, .expected = 1},
    example{.opcode = bc::opcode::lt, .left = 2, .right = 1, .expected = 0},
    example{.opcode = bc::opcode::le, .left = 2, .right = 2, .expected = 1},
    example{.opcode = bc::opcode::le, .left = 2, .right = 1, .expected = 0},
    example{.opcode = bc::opcode::gt, .left = 2, .right = 1, .expected = 1},
    example{.opcode = bc::opcode::gt, .left = 1, .right = 2, .expected = 0},
    example{.opcode = bc::opcode::ge, .left = 2, .right = 2, .expected = 1},
    example{.opcode = bc::opcode::ge, .left = 1, .right = 2, .expected = 0},
    example{.opcode = bc::opcode::eq, .left = -1, .right = -1, .expected = 1},
    example{.opcode = bc::opcode::eq, .left = -1, .right = 1, .expected = 0},
    example{.opcode = bc::opcode::ne, .left = -1, .right = 1, .expected = 1},
    example{.opcode = bc::opcode::ne, .left = -1, .right = -1, .expected = 0},
    example{.opcode = bc::opcode::logical_and, .left = -2, .right = 3, .expected = 1},
    example{.opcode = bc::opcode::logical_and, .left = -2, .right = 0, .expected = 0},
    example{.opcode = bc::opcode::logical_and, .left = 0, .right = 3, .expected = 0},
    example{.opcode = bc::opcode::logical_or, .left = 0, .right = 0, .expected = 0},
    example{.opcode = bc::opcode::logical_or, .left = -2, .right = 0, .expected = 1},
    example{.opcode = bc::opcode::logical_or, .left = 0, .right = 3, .expected = 1},
    example{
      .opcode = bc::opcode::add,
      .left = std::numeric_limits<std::int32_t>::max(),
      .right = 1,
      .expected = std::int64_t{std::numeric_limits<std::int32_t>::max()} + 1,
    },
  };

  for (const auto &[opcode, left, right, expected] : examples) {
    std::vector<std::byte> code;
    append(code, bc::constant{.value = left});
    append(code, bc::constant{.value = right});
    code.push_back(static_cast<std::byte>(opcode));
    append(code, bc::end{});
    loop machine({}, code);
    EXPECT_EQ(machine.run(), std::unexpected(error("unsupported instruction: end")));
    EXPECT_EQ(machine.stack_top(), (expected * 2) + 1) << static_cast<int>(opcode);
    EXPECT_FALSE(machine(bc::add{}));
  }
}

TEST(InterpreterExecution, BeginIgnoresArgsAndRejectsLocals) {
  constexpr std::array counts{-1, 0, 1, std::numeric_limits<std::int32_t>::max()};
  loop machine({}, {});
  for (const auto count : counts) {
    EXPECT_TRUE(machine(bc::begin{.n_args = count}));
    const status expected =
      count == 0 ? status{}
                 : std::unexpected(error("unsupported instruction: begin with nonzero n_locals"));
    EXPECT_EQ(machine(bc::begin{.n_locals = count}), expected);
  }

  EXPECT_FALSE(machine.stack_top());
}

TEST(InterpreterExecution, ReportsStackUnderflowWithoutConsumingOperands) {
  loop machine({}, {});
  EXPECT_EQ(machine.stack_top(), std::unexpected(error("stack underflow")));

  for (const auto has_operand : {false, true}) {
    if (has_operand) {
      ASSERT_TRUE(machine(bc::constant{.value = 1}));
    }

    EXPECT_EQ(machine(bc::add{}), std::unexpected(error("stack underflow")));
  }

  EXPECT_EQ(machine.stack_top(), 3);
}

TEST(InterpreterExecution, ReportsStackOverflow) {
  loop machine("abacaba", {});
  constexpr std::size_t capacity = (std::size_t{1024} * 1024) / sizeof(std::int64_t);
  for (std::size_t index = 0; index < capacity; ++index) {
    ASSERT_TRUE(machine(bc::constant{}));
  }

  EXPECT_EQ(machine(bc::constant{}), std::unexpected(error("stack overflow")));
  EXPECT_EQ(machine(bc::string{}), std::unexpected(error("stack overflow")));
  EXPECT_EQ(machine.stack_top(), runtime::box(0));
}

TEST(InterpreterExecution, ReportsDivisionAndRemainderByZero) {
  constexpr std::int32_t numerator = 42;
  for (const auto opcode : {bc::opcode::div, bc::opcode::mod}) {
    std::vector<std::byte> code;
    append(code, bc::constant{.value = numerator});
    append(code, bc::constant{});
    code.push_back(static_cast<std::byte>(opcode));
    append(code, bc::end{});
    loop machine({}, code);
    EXPECT_EQ(machine.run(), std::unexpected(error("division by zero")));
    EXPECT_EQ(machine.stack_top(), 1);
    EXPECT_TRUE(machine(bc::eq{}));
  }
}

} // namespace
} // namespace lamavm::interpreter
