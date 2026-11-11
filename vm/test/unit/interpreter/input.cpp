#include "bc/code_reader.hpp"
#include "bc/layout.hpp"
#include "core/exception.hpp"
#include "interpreter/boot.hpp"
#include "interpreter/loop.hpp"
#include "interpreter/state.hpp"

#include <gtest/gtest.h>

#include <array>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <format>
#include <ranges>
#include <span>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace lamavm::interpreter {
namespace {

using std::literals::string_view_literals::operator""sv;

constexpr std::size_t stack_size = 32;

TEST(InterpreterState, OwnsStorage) {
  constexpr std::int64_t initial_value = 0x42;
  constexpr std::int64_t changed_value = 0x24;
  for (const auto table : {""sv, "Std\0"sv, "a table larger than the small string buffer\0"sv}) {
    std::string strings(table);
    std::vector<std::int64_t> stack(stack_size, initial_value);
    const state machine(strings, {}, stack);
    strings.assign("changed");
    stack.clear();

    EXPECT_EQ(machine.string_table, table);
    ASSERT_EQ(machine.stack.size(), stack_size);
    EXPECT_EQ(machine.stack.front(), initial_value);
    machine.stack.front() = changed_value;
    EXPECT_EQ(machine.stack.front(), changed_value);
  }
}

TEST(InterpreterState, InitializesRegisters) {
  const state machine({}, {}, std::vector<std::int64_t>(stack_size));
  EXPECT_EQ(machine.ip, 0);
  EXPECT_EQ(machine.sp, stack_size);
  EXPECT_EQ(machine.bp, stack_size);
}

TEST(InterpreterState, AcceptsEmptyStorage) {
  const state machine({}, {}, {});
  EXPECT_TRUE(machine.string_table.empty());
  EXPECT_TRUE(machine.stack.empty());
  EXPECT_EQ(machine.ip, 0);
  EXPECT_EQ(machine.sp, 0);
  EXPECT_EQ(machine.bp, 0);
}

TEST(InterpreterState, PushesAndPopsValuesInReverseOrder) {
  constexpr std::array<std::int64_t, 3> values{1, -2, 3};
  state machine({}, {}, std::vector<std::int64_t>(values.size()));
  for (const auto value : values) {
    EXPECT_TRUE(machine.stack_push(value));
  }

  for (const auto value : std::views::reverse(values)) {
    EXPECT_EQ(machine.stack_top(), value);
    EXPECT_EQ(machine.stack_pop(), value);
  }

  EXPECT_EQ(std::pair(machine.sp, machine.bp), std::pair(values.size(), values.size()));
}

TEST(InterpreterState, StackOverflowPreservesExistingValue) {
  state machine({}, {}, std::vector<std::int64_t>(1));
  EXPECT_TRUE(machine.stack_push(1));
  EXPECT_EQ(machine.stack_push(2), std::unexpected(error("stack overflow")));
  EXPECT_EQ(machine.stack_top(), 1);
  EXPECT_EQ(machine.stack_pop(), 1);
}

TEST(InterpreterState, EmptyStackOperationsPreserveRegisters) {
  state machine({}, {}, {});
  EXPECT_EQ(machine.stack_push(1), std::unexpected(error("stack overflow")));
  EXPECT_EQ(machine.stack_pop(), std::unexpected(error("stack underflow")));
  EXPECT_EQ(machine.stack_top(), std::unexpected(error("stack underflow")));
  EXPECT_EQ(machine.sp, 0);
  EXPECT_EQ(machine.bp, 0);
}

TEST(InterpreterBoot, OwnsTableAndAcceptsOnlyStdImports) {
  boot listener;
  {
    std::string strings("Other\0Std\0StdExtra\0std\0\0"sv);
    ASSERT_TRUE(listener(strings));
    strings.assign("changed");
  }

  ASSERT_TRUE(listener(bc::import{.name_offset = 6}));
  for (const auto offset : {0, 10, 19, 23}) {
    const auto result = listener(bc::import{.name_offset = offset});
    ASSERT_FALSE(result) << offset;
    EXPECT_TRUE(result.error().starts_with("unsupported import:")) << result.error();
  }
}

TEST(InterpreterBoot, RejectsAllPublicSymbols) {
  boot listener;
  for (const auto kind : {bc::public_symbol::flag::function, bc::public_symbol::flag::global}) {
    const auto result = listener(bc::public_symbol{.flag = kind});
    ASSERT_FALSE(result);
    EXPECT_EQ(result.error(), "public symbols are not supported");
  }
}

TEST(InterpreterBoot, RunsValidatedCode) {
  boot listener;
  const auto bytes = "\x52\x00\x00\x00\x00\x00\x00\x00\x00\xff"sv;
  const auto result = listener(std::as_bytes(std::span(bytes)));
  ASSERT_FALSE(result);
  EXPECT_EQ(result.error(), (std::vector<error>{"unsupported instruction: eof"}));
}

TEST(InterpreterBoot, ValidatesEntireCodeBeforeExecution) {
  boot listener;
  const auto bytes = "\x52\x00\x00\x00\x00\x00\x00\x00\x00\x00\xff"sv;
  const auto result = listener(std::as_bytes(std::span(bytes)));
  ASSERT_FALSE(result);
  EXPECT_EQ(result.error(), (std::vector<error>{"offset 9: code: unknown opcode 0x00"}));
}

TEST(InterpreterLoop, RunsSuppliedCodeUntilError) {
  for (const auto bytes : {"\xff"sv, "\x16\x17\xff"sv}) {
    loop machine({}, std::as_bytes(std::span(bytes)));
    const auto result = machine.run();
    ASSERT_FALSE(result);
    EXPECT_EQ(
      result.error(),
      bytes.size() == 1 ? "unsupported instruction: eof" : "unsupported instruction: end");
  }
}

TEST(InterpreterLoop, NamesEveryUnsupportedInstruction) {
  using instruction = std::pair<bc::opcode, std::string_view>;
  constexpr std::array instructions{
    instruction{bc::opcode::string, "string"},
    instruction{bc::opcode::sexp, "sexp"},
    instruction{bc::opcode::sti, "sti"},
    instruction{bc::opcode::sta, "sta"},
    instruction{bc::opcode::jmp, "jmp"},
    instruction{bc::opcode::end, "end"},
    instruction{bc::opcode::ret, "ret"},
    instruction{bc::opcode::drop, "drop"},
    instruction{bc::opcode::dup, "dup"},
    instruction{bc::opcode::swap, "swap"},
    instruction{bc::opcode::elem, "elem"},
    instruction{bc::opcode::ld_glo, "ld_glo"},
    instruction{bc::opcode::ld_loc, "ld_loc"},
    instruction{bc::opcode::ld_arg, "ld_arg"},
    instruction{bc::opcode::ld_clo, "ld_clo"},
    instruction{bc::opcode::lda_glo, "lda_glo"},
    instruction{bc::opcode::lda_loc, "lda_loc"},
    instruction{bc::opcode::lda_arg, "lda_arg"},
    instruction{bc::opcode::lda_clo, "lda_clo"},
    instruction{bc::opcode::st_glo, "st_glo"},
    instruction{bc::opcode::st_loc, "st_loc"},
    instruction{bc::opcode::st_arg, "st_arg"},
    instruction{bc::opcode::st_clo, "st_clo"},
    instruction{bc::opcode::cjmp_z, "cjmp_z"},
    instruction{bc::opcode::cjmp_nz, "cjmp_nz"},
    instruction{bc::opcode::begin_closure, "begin_closure"},
    instruction{bc::opcode::closure, "closure"},
    instruction{bc::opcode::callc, "callc"},
    instruction{bc::opcode::call, "call"},
    instruction{bc::opcode::tag, "tag"},
    instruction{bc::opcode::array, "array"},
    instruction{bc::opcode::fail, "fail"},
    instruction{bc::opcode::fail_keep, "fail_keep"},
    instruction{bc::opcode::line, "line"},
    instruction{bc::opcode::patt_str_cmp, "patt_str_cmp"},
    instruction{bc::opcode::patt_string, "patt_string"},
    instruction{bc::opcode::patt_array, "patt_array"},
    instruction{bc::opcode::patt_sexp, "patt_sexp"},
    instruction{bc::opcode::patt_boxed, "patt_boxed"},
    instruction{bc::opcode::patt_unboxed, "patt_unboxed"},
    instruction{bc::opcode::patt_closure, "patt_closure"},
    instruction{bc::opcode::barray, "barray"},
    instruction{bc::opcode::eof, "eof"},
  };

  loop listener(std::string{}, {});
  for (const auto &[opcode, name] : instructions) {
    std::array<std::byte, sizeof(bc::begin_closure)> code{};
    code.front() = static_cast<std::byte>(opcode);
    const auto result = bc::read_op(std::span<const std::byte>(code), listener);
    ASSERT_FALSE(result) << name;
    EXPECT_EQ(result.error(), std::format("unsupported instruction: {}", name));
  }
}

} // namespace
} // namespace lamavm::interpreter
