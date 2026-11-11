#include "bc/code_reader.hpp"
#include "bc/layout.hpp"
#include "core/exception.hpp"
#include "interpreter/boot.hpp"
#include "interpreter/loop.hpp"
#include "interpreter/stack.hpp"
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
#include <thread>
#include <utility>
#include <vector>

namespace lamavm::interpreter {
namespace {

using std::literals::string_view_literals::operator""sv;

TEST(InterpreterState, OwnsStringTable) {
  for (const auto table : {""sv, "Std\0"sv, "a table larger than the small string buffer\0"sv}) {
    std::string strings(table);
    const state machine(strings, {});
    strings.assign("changed");
    EXPECT_EQ(machine.string_table, table);
  }
}

TEST(InterpreterState, InitializesRegisters) {
  const state machine({}, {});
  EXPECT_EQ(machine.ip, 0);
  EXPECT_EQ(machine.bp, stack::capacity());
  EXPECT_EQ(stack::size(), 0);
}

TEST(InterpreterStack, PushesAndPopsValuesInReverseOrder) {
  constexpr std::array<std::int64_t, 3> values{1, -2, 3};
  const state machine({}, {});
  for (const auto value : values) {
    EXPECT_TRUE(stack::push(value));
  }

  for (const auto value : std::views::reverse(values)) {
    EXPECT_EQ(stack::top(), value);
    EXPECT_EQ(stack::pop(), value);
  }

  EXPECT_EQ(stack::size(), 0);
}

TEST(InterpreterStack, EmptyStackOperationsPreserveRegisters) {
  const state machine({}, {});
  EXPECT_EQ(stack::pop(), std::unexpected(error("stack underflow")));
  EXPECT_EQ(stack::top(), std::unexpected(error("stack underflow")));
  EXPECT_EQ(stack::drop(1), std::unexpected(error("stack underflow")));
  EXPECT_EQ(stack::size(), 0);
  EXPECT_EQ(machine.bp, stack::capacity());
}

TEST(InterpreterStack, DropsOnlyAvailableValues) {
  const state machine({}, {});
  ASSERT_TRUE(stack::push(1));
  EXPECT_EQ(stack::drop(2), std::unexpected(error("stack underflow")));
  EXPECT_EQ(stack::top(), 1);
  EXPECT_TRUE(stack::drop(1));
  EXPECT_EQ(stack::size(), 0);
}

TEST(InterpreterState, RejectsConcurrentConstruction) {
  const state machine({}, {});
  ASSERT_TRUE(stack::push(1));
  bool is_rejected = false;
  std::jthread contender([&is_rejected] -> void {
    try {
      const state other({}, {});
    } catch (const exception &) {
      is_rejected = true;
    }
  });
  contender.join();
  EXPECT_TRUE(is_rejected);
  EXPECT_EQ(stack::top(), 1);
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
