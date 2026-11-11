#include "bc/layout.hpp"
#include "core/exception.hpp"
#include "interpreter/loop.hpp"
#include "interpreter/stack.hpp"
#include "runtime/box.hpp"
#include "runtime/sexp.hpp"

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

auto sexp_words(std::int64_t value, std::size_t fields) -> std::span<const std::int64_t> {
  return {std::bit_cast<const std::int64_t *>(value), fields + 1};
}

auto grow_chain(loop &machine, std::int32_t depth) -> status {
  for (std::int32_t index = 0; index < depth; ++index) {
    if (const auto result = machine(bc::constant{.value = index}); !result) {
      return result;
    }

    if (const auto result = machine(bc::sexp{.n_fields = 2}); !result) {
      return result;
    }
  }

  return {};
}

auto make_pair(loop &machine, std::int32_t left, std::int32_t right) -> status {
  if (const auto result = machine(bc::constant{.value = left}); !result) {
    return result;
  }

  if (const auto result = machine(bc::constant{.value = right}); !result) {
    return result;
  }

  return machine(bc::sexp{.n_fields = 2});
}

void expect_chain(std::int64_t value, std::int32_t depth) {
  for (std::int32_t index = depth; index > 0; --index) {
    const auto words = sexp_words(value, 2);
    EXPECT_EQ(words.front(), runtime::unbox(runtime::tag_hash("Pair")));
    EXPECT_EQ(words.subspan(2).front(), runtime::box(index - 1));
    value = words.subspan(1).front();
  }

  EXPECT_EQ(sexp_words(value, 0).front(), runtime::unbox(runtime::tag_hash("Nil")));
}

auto replace_objects(loop &machine, std::int32_t attempts) -> status {
  for (std::int32_t index = 0; index < attempts; ++index) {
    if (const auto result = machine(bc::constant{.value = index}); !result) {
      return result;
    }

    if (const auto result = machine(bc::sexp{.n_fields = 1}); !result) {
      return result;
    }

    EXPECT_EQ(sexp_words(*stack::top(), 1).subspan(1).front(), runtime::box(index));

    if (const auto result = machine(bc::constant{}); !result) {
      return result;
    }

    if (const auto result = machine(bc::eq{}); !result) {
      return result;
    }
  }

  return {};
}

void fill_stack(loop &machine) {
  for (std::size_t index = 0; index < stack::capacity(); ++index) {
    ASSERT_TRUE(machine(bc::constant{}));
  }
}

TEST(InterpreterExecution, ConstructsSexpInSourceOrderAndAdvancesInstructionPointer) {
  constexpr std::int32_t left = -10;
  constexpr std::int32_t right = 20;
  std::vector<std::byte> code;
  append(code, bc::constant{.value = left});
  append(code, bc::constant{.value = right});
  append(code, bc::sexp{.tag_offset = 4, .n_fields = 2});
  append(code, bc::end{});
  loop machine(std::string("Std\0Pair", sizeof("Std\0Pair")), code);
  EXPECT_EQ(machine.run(), std::unexpected(error("unsupported instruction: end")));
  ASSERT_TRUE(stack::top());
  const auto words = sexp_words(*stack::top(), 2);
  EXPECT_EQ(words.front(), runtime::unbox(runtime::tag_hash("Pair")));
  EXPECT_EQ(words.subspan(1).front(), runtime::box(left));
  EXPECT_EQ(words.subspan(2).front(), runtime::box(right));
  EXPECT_EQ(
    machine(bc::sexp{.tag_offset = 4, .n_fields = 2}),
    std::unexpected(error("stack underflow")));
}

TEST(InterpreterExecution, ConstructsNullaryAndNestedSexps) {
  loop machine(std::string("Nil\0Pair", sizeof("Nil\0Pair")), {});
  ASSERT_TRUE(machine(bc::sexp{}));
  ASSERT_TRUE(stack::top());
  EXPECT_EQ(sexp_words(*stack::top(), 0).front(), runtime::unbox(runtime::tag_hash("Nil")));
  ASSERT_TRUE(machine(bc::constant{.value = 42}));
  ASSERT_TRUE(machine(bc::sexp{.tag_offset = 4, .n_fields = 2}));
  const auto words = sexp_words(*stack::top(), 2);
  EXPECT_EQ(words.front(), runtime::unbox(runtime::tag_hash("Pair")));
  EXPECT_EQ(
    sexp_words(words.subspan(1).front(), 0).front(),
    runtime::unbox(runtime::tag_hash("Nil")));
  EXPECT_EQ(words.subspan(2).front(), runtime::box(42));
}

TEST(InterpreterExecution, RejectsInvalidSexpOperandsWithoutConsumingValues) {
  loop machine("Pair", {});
  ASSERT_TRUE(machine(bc::constant{.value = 42}));
  EXPECT_EQ(machine(bc::sexp{.n_fields = -1}), std::unexpected(error("negative sexp field count")));
  EXPECT_EQ(machine(bc::sexp{.n_fields = 2}), std::unexpected(error("stack underflow")));

  for (const auto offset : {-1, 4, std::numeric_limits<std::int32_t>::max()}) {
    EXPECT_EQ(
      machine(bc::sexp{.tag_offset = offset, .n_fields = 1}),
      std::unexpected(error("invalid sexp tag offset")));
  }

  EXPECT_EQ(stack::top(), runtime::box(42));
}

TEST(InterpreterExecution, CollectionUpdatesNestedArgumentsAndUnrelatedStackValues) {
  loop machine(std::string("Pair\0Nil", sizeof("Pair\0Nil")), {});
  ASSERT_TRUE(make_pair(machine, -1, 42));
  const auto initial = *stack::top();
  ASSERT_TRUE(machine(bc::sexp{.tag_offset = 5}));

  constexpr std::int32_t depth = 256;
  ASSERT_TRUE(grow_chain(machine, depth));

  expect_chain(*stack::top(), depth);
  ASSERT_TRUE(machine(bc::sexp{.n_fields = 2}));
  const auto words = sexp_words(*stack::top(), 2);
  EXPECT_NE(words.subspan(1).front(), initial);
  EXPECT_EQ(sexp_words(words.subspan(1).front(), 2).subspan(1).front(), runtime::box(-1));
  EXPECT_EQ(sexp_words(words.subspan(1).front(), 2).subspan(2).front(), runtime::box(42));
}

TEST(InterpreterExecution, RejectsSecondInterpreterWithoutDisturbingTheOwner) {
  loop first("Pair", {});
  ASSERT_TRUE(make_pair(first, -1, 42));
  EXPECT_THROW((loop({}, {})), exception);
  const auto words = sexp_words(*stack::top(), 2);
  EXPECT_EQ(words.subspan(1).front(), runtime::box(-1));
  EXPECT_EQ(words.subspan(2).front(), runtime::box(42));
  ASSERT_TRUE(first(bc::constant{}));
  ASSERT_TRUE(first(bc::sexp{.n_fields = 2}));
}

TEST(InterpreterExecution, ReinitializesHeapAndReclaimsPoppedObjects) {
  constexpr std::int32_t attempts = 256;
  for (const auto is_reinitialized : {false, true}) {
    loop machine("Pair", {});
    EXPECT_FALSE(stack::top());
    ASSERT_TRUE(replace_objects(machine, attempts));

    EXPECT_EQ(stack::top(), runtime::box(0)) << is_reinitialized;
  }
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
    EXPECT_EQ(stack::top(), (2 * (initial + static_cast<int>(is_addition))) + 1);
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
    EXPECT_EQ(stack::top(), (static_cast<std::int64_t>(value) * 2) + 1);
  }
}

TEST(InterpreterExecution, PushesPointersIntoStringTable) {
  std::vector<std::byte> code;
  append(code, bc::begin{});
  append(code, bc::string{.string_offset = 4});
  append(code, bc::end{});
  loop machine(std::string("Std\0abacaba", sizeof("Std\0abacaba")), code);
  EXPECT_EQ(machine.run(), std::unexpected(error("unsupported instruction: end")));
  const auto top = stack::top();
  ASSERT_TRUE(top);
  const std::string_view value(std::bit_cast<const char *>(*top));
  EXPECT_EQ(value, "abacaba");

  ASSERT_TRUE(machine(bc::string{.string_offset = 5}));
  EXPECT_EQ(stack::top(), std::bit_cast<std::int64_t>(value.substr(1).data()));

  ASSERT_TRUE(machine(bc::string{.string_offset = 11}));
  EXPECT_EQ(stack::top(), std::bit_cast<std::int64_t>(value.substr(value.size()).data()));
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
    EXPECT_EQ(stack::top(), (expected * 2) + 1) << static_cast<int>(opcode);
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

  EXPECT_FALSE(stack::top());
}

TEST(InterpreterExecution, ReportsStackUnderflowWithoutConsumingOperands) {
  loop machine({}, {});
  EXPECT_EQ(stack::top(), std::unexpected(error("stack underflow")));

  for (const auto has_operand : {false, true}) {
    if (has_operand) {
      ASSERT_TRUE(machine(bc::constant{.value = 1}));
    }

    EXPECT_EQ(machine(bc::add{}), std::unexpected(error("stack underflow")));
  }

  EXPECT_EQ(stack::top(), 3);
}

TEST(InterpreterExecution, ReportsStackOverflow) {
  loop machine("abacaba", {});
  fill_stack(machine);

  EXPECT_EQ(machine(bc::constant{}), std::unexpected(error("stack overflow")));
  EXPECT_EQ(machine(bc::string{}), std::unexpected(error("stack overflow")));
  EXPECT_EQ(machine(bc::sexp{}), std::unexpected(error("stack overflow")));
  EXPECT_EQ(stack::top(), runtime::box(0));
  ASSERT_TRUE(machine(bc::sexp{.n_fields = 1}));
  EXPECT_EQ(sexp_words(*stack::top(), 1).subspan(1).front(), runtime::box(0));
}

TEST(InterpreterExecution, CollectionUpdatesTheTopOfAFullSingletonStack) {
  loop machine("Pair", {});
  fill_stack(machine);
  constexpr std::int32_t depth = 256;
  for (std::int32_t index = 0; index < depth; ++index) {
    ASSERT_TRUE(machine(bc::sexp{.n_fields = 1}));
  }

  auto value = *stack::top();
  for (std::int32_t index = 0; index < depth; ++index) {
    value = sexp_words(value, 1).subspan(1).front();
  }

  EXPECT_EQ(value, runtime::box(0));
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
    EXPECT_EQ(stack::top(), 1);
    EXPECT_TRUE(machine(bc::eq{}));
  }
}

} // namespace
} // namespace lamavm::interpreter
