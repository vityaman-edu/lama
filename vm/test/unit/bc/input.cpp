#include "bc/input.hpp"
#include "bc/lamac_ds.hpp"
#include "bc/layout.hpp"
#include "bc/listener.hpp"
#include "core/exception.hpp"

#include <gtest/gtest.h>

#include <cstddef>
#include <cstdint>
#include <limits>
#include <span>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

namespace lamavm::bc {
namespace {

using std::literals::string_view_literals::operator""sv;

void append_int(std::string &bytes, std::int32_t value) {
  auto bits = static_cast<std::uint32_t>(value);
  for (std::size_t i = 0; i < sizeof(value); ++i) {
    bytes += static_cast<char>(bits & std::numeric_limits<std::uint8_t>::max());
    bits >>= static_cast<unsigned>(std::numeric_limits<std::uint8_t>::digits);
  }
}

auto make_file(std::string_view code, std::string_view table) -> std::string {
  std::string result = "LaMa";
  append_int(result, 1);
  append_int(result, static_cast<std::int32_t>(table.size()));
  append_int(result, 0);
  append_int(result, 0);
  append_int(result, 0);
  result += table;
  result += code;

  return result;
}

auto make_file(std::string_view code) -> std::string {
  return make_file(code, {});
}

struct listener_events {
  std::vector<std::int32_t> constants;
  std::vector<closure::capture> captures;
  std::int32_t closure_target = 0;
  std::size_t error_offset = 0;
  std::string error_message;
  bool finished = false;
};

class recording_listener final : public base_listener {
public:
  using base_listener::operator();

  [[nodiscard]] auto events() const -> const listener_events & {
    return result;
  }

  void operator()(const constant &instruction) override {
    result.constants.push_back(std::int32_t{instruction.value});
  }

  void operator()(const closure &instruction, std::span<const closure::capture> values) override {
    result.closure_target = instruction.target;
    result.captures.assign(values.begin(), values.end());
  }

  void operator()(const eof &instruction) override {
    (void)instruction;

    result.finished = true;
  }

  void operator()(std::size_t offset, std::string_view message) override {
    result.error_offset = offset;
    result.error_message = message;
  }

private:
  listener_events result;
};

TEST(BytecodeInput, SignedOperandDoesNotTerminateCode) {
  std::istringstream input(make_file("\x10\xff\xff\xff\xff\x10\x2a\x00\x00\x00\xff"sv));
  recording_listener listener;
  ASSERT_TRUE(read_file(input, listener));
  EXPECT_EQ(listener.events().constants, (std::vector<std::int32_t>{-1, 42}));
  EXPECT_TRUE(listener.events().finished);
}

TEST(BytecodeInput, ClosureCapturesPreserveFollowingInstruction) {
  std::istringstream input(make_file(
    "\x54\x2a\x00\x00\x00\x02\x00\x00\x00\x01\x07\x00\x00\x00\x03\x09"
    "\x00\x00\x00\x10\x05\x00\x00\x00\xff"sv));
  recording_listener listener;
  ASSERT_TRUE(read_file(input, listener));
  EXPECT_EQ(listener.events().closure_target, 42);
  ASSERT_EQ(listener.events().captures.size(), 2);
  EXPECT_EQ(listener.events().captures.at(0).kind, closure::capture::kind::local);
  EXPECT_EQ(std::int32_t{listener.events().captures.at(0).index}, 7);
  EXPECT_EQ(listener.events().captures.at(1).kind, closure::capture::kind::captured);
  EXPECT_EQ(std::int32_t{listener.events().captures.at(1).index}, 9);
  EXPECT_EQ(listener.events().constants, (std::vector<std::int32_t>{5}));
  EXPECT_TRUE(listener.events().finished);
}

TEST(BytecodeInput, EveryTruncationFailsWithoutEof) {
  const auto complete = make_file("\x54\x00\x00\x00\x00\x01\x00\x00\x00\x02\x03\x00\x00\x00\xff"sv);
  for (std::size_t size = 0; size < complete.size(); ++size) {
    std::istringstream input(complete.substr(0, size));
    recording_listener listener;
    EXPECT_FALSE(read_file(input, listener)) << size;
    EXPECT_FALSE(listener.events().finished) << size;
    EXPECT_FALSE(listener.events().error_message.empty()) << size;
  }
}

TEST(BytecodeInput, UnknownOpcodeReportsItsFileOffset) {
  std::istringstream input(make_file("\x00\xff"sv));
  recording_listener listener;
  EXPECT_FALSE(read_file(input, listener));
  EXPECT_EQ(listener.events().error_offset, sizeof(header));
  EXPECT_EQ(listener.events().error_message, "code: unknown opcode 0x00");
  EXPECT_FALSE(listener.events().finished);
}

TEST(BytecodeInput, InvalidCaptureCountAndKindFail) {
  for (const auto &bytes : {
         make_file("\x54\x00\x00\x00\x00\xff\xff\xff\xff\xff"sv),
         make_file("\x54\x00\x00\x00\x00\x01\x00\x00\x00\x04\x00\x00\x00\x00\xff"sv),
       }) {
    std::istringstream input(bytes);
    recording_listener listener;
    EXPECT_FALSE(read_file(input, listener));
    EXPECT_TRUE(listener.events().captures.empty());
    EXPECT_FALSE(listener.events().finished);
    EXPECT_FALSE(listener.events().error_message.empty());
  }
}

TEST(LamacDump, ConstantsExternalCallsAndEof) {
  std::istringstream input(make_file(
    "\x52\x02\x00\x00\x00\x00\x00\x00\x00\x10\x2a\x00\x00\x00\x56\xff"
    "\xff\xff\xff\x01\x00\x00\x00\x16\xff"sv,
    "write\0"sv));
  std::ostringstream output;
  lamac_ds_listener listener(output);
  ASSERT_TRUE(read_file(input, listener));
  EXPECT_EQ(
    output.str(),
    "LABEL (\"0x00000000\")\n"
    "BEGIN (\"0x00000000\", 2, 0, [], [], [])\n"
    "CONST (42)\n"
    "CALL (\"write\", 1, false)\n"
    "END\n");
}

TEST(LamacDump, StringsUseOcamlEscaping) {
  std::istringstream input(make_file("\x11\x00\x00\x00\x00\xff"sv, "a\n\t\"\\\x01\0"sv));
  std::ostringstream output;
  lamac_ds_listener listener(output);
  ASSERT_TRUE(read_file(input, listener));
  EXPECT_EQ(output.str(), "STRING (\"a\\n\\t\\\"\\\\\\001\")\n");
}

TEST(LamacDump, MinExternalReferenceDoesNotOverflow) {
  std::istringstream input(make_file("\x56\x00\x00\x00\x80\x00\x00\x00\x00\xff"sv));
  std::ostringstream output;
  lamac_ds_listener listener(output);
  try {
    EXPECT_TRUE(read_file(input, listener));
    ADD_FAILURE() << "Expected lamavm::exception";
  } catch (const lamavm::exception &e) {
    EXPECT_STREQ(e.what(), "invalid string offset 2147483647");
  }
}

} // namespace
} // namespace lamavm::bc
