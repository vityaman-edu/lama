#include "bc/code_listener_nop.hpp"
#include "bc/code_reader.hpp"
#include "bc/disassembly/legacy.hpp"
#include "bc/disassembly/modern.hpp"
#include "bc/file_listener.hpp"
#include "bc/file_listener_seq.hpp"
#include "bc/file_reader.hpp"
#include "bc/layout.hpp"
#include "core/exception.hpp"
#include "interpreter/boot.hpp"

#include <gtest/gtest.h>

#include <cstddef>
#include <cstdint>
#include <expected>
#include <format>
#include <limits>
#include <memory>
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

auto code_listing(std::string_view output) -> std::string {
  constexpr std::string_view marker = "\ncode:\n";
  const auto start = output.find(marker);
  return std::string(output.substr(start == std::string_view::npos ? 0 : start + marker.size()));
}

struct listener_events {
  const void *constant_address = nullptr;
  const void *closure_address = nullptr;
  const void *captures_address = nullptr;
  const void *eof_address = nullptr;
  std::vector<std::int32_t> constants;
  std::vector<closure::capture> captures;
  std::int32_t closure_target = 0;
  bool is_finished = false;
};

class recording_listener final : public code_listener_nop {
public:
  using code_listener_nop::operator();

  [[nodiscard]] auto events() const -> const listener_events & {
    return result;
  }

  void operator()(const constant &op) override {
    result.constant_address = &op;
    result.constants.push_back(std::int32_t{op.value});
  }

  void operator()(const closure &op, std::span<const closure::capture> values) override {
    result.closure_address = &op;
    result.captures_address = values.data();
    result.closure_target = op.target;
    result.captures.assign(values.begin(), values.end());
  }

  void operator()(const eof &op) override {
    result.eof_address = &op;
    result.is_finished = true;
  }

private:
  listener_events result;
};

struct file_events {
  header file_header;
  std::vector<std::string_view> order;
  std::string strings;
  std::vector<std::int32_t> imports;
  std::vector<std::int32_t> symbols;
  std::vector<std::byte> code;
};

class recording_file_listener final : public file_listener {
public:
  using file_listener::operator();

  [[nodiscard]] auto events() const -> const file_events & {
    return result;
  }

  void operator()(const header &op) override {
    result.file_header = op;
    result.order.emplace_back("header");
  }

  void operator()(std::string_view table) override {
    result.order.emplace_back("strings");
    result.strings = table;
  }

  void operator()(const import &op) override {
    result.order.emplace_back("import");
    result.imports.push_back(std::int32_t{op.name_offset});
  }

  void operator()(const public_symbol &op) override {
    result.order.emplace_back("symbol");
    result.symbols.push_back(std::int32_t{op.code_offset});
  }

  auto operator()(std::span<const std::byte> code)
    -> std::expected<void, std::vector<std::string>> override {
    result.order.emplace_back("code");
    result.code.assign(code.begin(), code.end());
    return {};
  }

private:
  file_events result;
};

TEST(BytecodeFile, MetadataPrecedesCodeSpan) {
  const auto table = "external\0function\0"sv;
  std::string bytes = "LaMa";
  append_int(bytes, 1);
  append_int(bytes, static_cast<std::int32_t>(table.size()));
  append_int(bytes, 0);
  append_int(bytes, 1);
  append_int(bytes, 1);
  bytes += table;
  append_int(bytes, 0);
  constexpr std::int32_t function_name_offset = 9;
  append_int(bytes, function_name_offset);
  constexpr std::int32_t function_address = 42;
  append_int(bytes, function_address);
  bytes += '\0';
  bytes += "\x10\xff\xff\xff\xff\xff"sv;

  std::istringstream input(bytes);
  recording_file_listener listener;
  ASSERT_TRUE(read_file(input, listener));
  EXPECT_EQ(
    listener.events().order,
    (std::vector<std::string_view>{"header", "strings", "import", "symbol", "code"}));
  EXPECT_EQ(listener.events().strings, table);
  EXPECT_EQ(listener.events().imports, (std::vector<std::int32_t>{0}));
  EXPECT_EQ(listener.events().symbols, (std::vector<std::int32_t>{42}));
  EXPECT_EQ(
    listener.events().code,
    (std::vector<std::byte>{
      std::byte{0x10},
      std::byte{0xff},
      std::byte{0xff},
      std::byte{0xff},
      std::byte{0xff},
      std::byte{0xff},
    }));
}

TEST(BytecodeFile, EveryMetadataTruncationReportsFileOffset) {
  const auto complete = make_file({}, "table\0"sv);
  for (std::size_t size = 0; size < complete.size(); ++size) {
    std::istringstream input(complete.substr(0, size));
    recording_file_listener listener;
    const auto result = read_file(input, listener);
    ASSERT_FALSE(result) << size;
    ASSERT_EQ(result.error().size(), 1) << size;
    EXPECT_TRUE(result.error().front().starts_with(std::format("offset {}: ", size))) << size;
  }
}

TEST(BytecodeFile, CollectsRecoverableMetadataErrors) {
  std::string bytes = "LaMa";
  append_int(bytes, 1);
  append_int(bytes, 1);
  append_int(bytes, 0);
  append_int(bytes, 2);
  append_int(bytes, 1);
  bytes += '\0';
  append_int(bytes, 1);
  append_int(bytes, -1);
  append_int(bytes, 0);
  append_int(bytes, 0);
  bytes += '\2';
  bytes += '\xff';
  std::istringstream input(bytes);
  recording_file_listener listener;
  const auto result = read_file(input, listener);
  ASSERT_FALSE(result);
  EXPECT_EQ(
    result.error(),
    (std::vector<std::string>{
      "offset 25: import: expected name_offset in [0, 1), but got 1",
      "offset 29: import: expected name_offset in [0, 1), but got -1",
      "offset 41: symbol: expected flag in [0, 2), but got 2",
    }));
  EXPECT_EQ(listener.events().order, (std::vector<std::string_view>{"header", "strings"}));
}

TEST(BytecodeFile, FramingFailurePreservesEarlierMetadataErrors) {
  std::string bytes = "LaMa";
  append_int(bytes, 1);
  append_int(bytes, 1);
  append_int(bytes, 0);
  append_int(bytes, 2);
  append_int(bytes, 0);
  bytes += '\0';
  append_int(bytes, 1);
  bytes += '\0';
  std::istringstream input(bytes);
  recording_file_listener listener;
  const auto result = read_file(input, listener);
  ASSERT_FALSE(result);
  EXPECT_EQ(
    result.error(),
    (std::vector<std::string>{
      "offset 25: import: expected name_offset in [0, 1), but got 1",
      "offset 30: import: expected 4 bytes, but got 1",
    }));
}

TEST(BytecodeFile, CodeValidationIsDelegatedToFileListener) {
  std::istringstream input(make_file("\x10\x01\x00\x00\x00\x00\xff"sv));
  recording_file_listener listener;
  const auto result = read_file(input, listener);
  ASSERT_TRUE(result);
  EXPECT_EQ(listener.events().order, (std::vector<std::string_view>{"header", "strings", "code"}));
}

TEST(InterpreterBoot, RejectsMalformedCode) {
  for (const auto code : {
         ""sv,
         "\x10\x01\x00\x00\x00"sv,
         "\x10\x01\x00"sv,
         "\x54\x00\x00\x00\x00\xff\xff\xff\xff\xff"sv,
         "\x54\x00\x00\x00\x00\x01\x00\x00\x00\x04\x00\x00\x00\x00\xff"sv,
         "\x54\x00\x00\x00\x00\x01\x00\x00\x00\x01\x00\xff"sv,
       }) {
    std::istringstream input(make_file(code));
    interpreter::boot listener;
    const auto result = read_file(input, listener);
    ASSERT_FALSE(result);
    EXPECT_EQ(result.error().size(), 1);
  }
}

TEST(FileListenerSeq, CallsBothListenersAndCombinesCodeErrors) {
  std::istringstream input(make_file("\x00\xff"sv));
  file_listener_seq listener(
    std::make_unique<interpreter::boot>(),
    std::make_unique<interpreter::boot>());
  const auto result = read_file(input, listener);
  ASSERT_FALSE(result);
  EXPECT_EQ(
    result.error(),
    (std::vector<std::string>{
      "offset 0: code: unknown opcode 0x00",
      "offset 0: code: unknown opcode 0x00",
    }));
}

TEST(BytecodeOp, AcceptsConcreteListenerWithoutInheritance) {
  std::size_t calls = 0;
  auto listener = [&calls](const auto &...) -> void { ++calls; };
  for (const auto input : {
         "\x10\x2a\x00\x00\x00"sv,
         "\x54\x2a\x00\x00\x00\x01\x00\x00\x00\x01\x07\x00\x00\x00"sv,
       }) {
    const auto bytes = std::as_bytes(std::span(input));
    ASSERT_TRUE(read_op(bytes, listener));
    ASSERT_TRUE((read_op<decltype(listener), true>(bytes, listener)));
    ASSERT_TRUE(read_op_unsafe(bytes, listener));
  }

  EXPECT_EQ(calls, 6);
}

TEST(BytecodeOp, ReadsOnlyPrefixWithoutRequiringEof) {
  const auto input = "\x10\x2a\x00\x00\x00\x00\xff"sv;
  const auto bytes = std::as_bytes(std::span(input));
  for (const auto size : {sizeof(constant), bytes.size()}) {
    recording_listener listener;
    ASSERT_TRUE(read_op(bytes.first(size), listener));
    EXPECT_EQ(listener.events().constants, (std::vector<std::int32_t>{42}));
    EXPECT_EQ(listener.events().constant_address, bytes.data());
    EXPECT_FALSE(listener.events().is_finished);
  }
}

TEST(BytecodeOp, CallerAdvancesSpanPastClosureCaptures) {
  const auto input = "\x54\x2a\x00\x00\x00\x01\x00\x00\x00\x01\x07\x00\x00\x00"
                     "\x10\x05\x00\x00\x00\xff"sv;
  auto bytes = std::as_bytes(std::span(input));
  recording_listener listener;
  ASSERT_TRUE(read_op(bytes, listener));
  EXPECT_EQ(listener.events().closure_address, bytes.data());
  EXPECT_EQ(listener.events().closure_target, 42);
  ASSERT_EQ(listener.events().captures.size(), 1);
  EXPECT_EQ(listener.events().captures.front().kind, closure::capture::kind::local);
  EXPECT_EQ(std::int32_t{listener.events().captures.front().index}, 7);
  EXPECT_EQ(listener.events().captures_address, bytes.subspan(sizeof(closure)).data());
  EXPECT_TRUE(listener.events().constants.empty());
  EXPECT_FALSE(listener.events().is_finished);

  bytes =
    bytes.subspan(sizeof(closure) + (listener.events().captures.size() * sizeof(closure::capture)));
  ASSERT_TRUE(read_op(bytes, listener));
  EXPECT_EQ(listener.events().constants, (std::vector<std::int32_t>{5}));
  EXPECT_EQ(listener.events().constant_address, bytes.data());
  EXPECT_FALSE(listener.events().is_finished);

  bytes = bytes.subspan(sizeof(constant));
  ASSERT_TRUE(read_op(bytes, listener));
  EXPECT_EQ(listener.events().eof_address, bytes.data());
  EXPECT_TRUE(listener.events().is_finished);
}

TEST(BytecodeOp, UnsafePreservesPackedClosureViews) {
  const auto input = "\x54\x2a\x00\x00\x00\x01\x00\x00\x00\x01\x07\x00\x00\x00"
                     "\x10\x05\x00\x00\x00\xff"sv;
  const auto bytes = std::as_bytes(std::span(input));
  recording_listener listener;
  ASSERT_TRUE(read_op_unsafe(bytes, listener));
  EXPECT_EQ(listener.events().closure_address, bytes.data());
  EXPECT_EQ(listener.events().closure_target, 42);
  EXPECT_EQ(listener.events().captures_address, bytes.subspan(sizeof(closure)).data());
  ASSERT_EQ(listener.events().captures.size(), 1);
  EXPECT_EQ(listener.events().captures.front().kind, closure::capture::kind::local);
  EXPECT_EQ(std::int32_t{listener.events().captures.front().index}, 7);
  EXPECT_TRUE(listener.events().constants.empty());
  EXPECT_FALSE(listener.events().is_finished);
}

TEST(BytecodeOp, EmptyUnknownAndTruncatedPrefixesReturnErrors) {
  recording_listener listener;
  auto result = read_op({}, listener);
  ASSERT_FALSE(result);
  EXPECT_EQ(result.error(), "offset 0: code: expected 1 bytes, but got 0");

  const auto unknown = "\x00\xff"sv;
  result = read_op(std::as_bytes(std::span(unknown)), listener);
  ASSERT_FALSE(result);
  EXPECT_EQ(result.error(), "offset 0: code: unknown opcode 0x00");

  const auto truncated = "\x10\x01\x00"sv;
  result = read_op(std::as_bytes(std::span(truncated)), listener);
  ASSERT_FALSE(result);
  EXPECT_EQ(result.error(), "offset 3: code: expected 5 bytes, but got 3");
  EXPECT_TRUE(listener.events().constants.empty());
  EXPECT_FALSE(listener.events().is_finished);
}

TEST(BytecodeOp, EveryClosureTruncationFailsBeforeCallback) {
  const auto complete = "\x54\x00\x00\x00\x00\x01\x00\x00\x00\x02\x03\x00\x00\x00"sv;
  for (std::size_t size = 0; size < complete.size(); ++size) {
    recording_listener listener;
    const auto input = complete.substr(0, size);
    const auto result = read_op(std::as_bytes(std::span(input)), listener);
    ASSERT_FALSE(result) << size;
    EXPECT_FALSE(result.error().empty()) << size;
    EXPECT_EQ(listener.events().closure_address, nullptr) << size;
  }
}

TEST(BytecodeInput, AcceptsMutableSpanWithoutChangingBytes) {
  std::string code("\x10\xff\xff\xff\xff\xff"sv);
  const auto original = code;
  recording_listener listener;
  ASSERT_TRUE(read_ops(std::as_writable_bytes(std::span(code)), listener));
  EXPECT_EQ(code, original);
  EXPECT_EQ(listener.events().constants, (std::vector<std::int32_t>{-1}));
}

TEST(BytecodeInput, SignedOperandDoesNotTerminateCode) {
  const auto input = "\x10\xff\xff\xff\xff\x10\x2a\x00\x00\x00\xff"sv;
  recording_listener listener;
  ASSERT_TRUE(read_ops(std::as_bytes(std::span(input)), listener));
  EXPECT_EQ(listener.events().constants, (std::vector<std::int32_t>{-1, 42}));
  EXPECT_TRUE(listener.events().is_finished);
}

TEST(BytecodeInput, ClosureCapturesPreserveFollowingInstruction) {
  const auto input = "\x54\x2a\x00\x00\x00\x02\x00\x00\x00\x01\x07\x00\x00\x00\x03\x09"
                     "\x00\x00\x00\x10\x05\x00\x00\x00\xff"sv;
  recording_listener listener;
  ASSERT_TRUE(read_ops(std::as_bytes(std::span(input)), listener));
  EXPECT_EQ(listener.events().closure_target, 42);
  ASSERT_EQ(listener.events().captures.size(), 2);
  EXPECT_EQ(listener.events().captures.at(0).kind, closure::capture::kind::local);
  EXPECT_EQ(std::int32_t{listener.events().captures.at(0).index}, 7);
  EXPECT_EQ(listener.events().captures.at(1).kind, closure::capture::kind::captured);
  EXPECT_EQ(std::int32_t{listener.events().captures.at(1).index}, 9);
  EXPECT_EQ(listener.events().constants, (std::vector<std::int32_t>{5}));
  EXPECT_TRUE(listener.events().is_finished);
}

TEST(BytecodeInput, PackedViewsPointIntoOriginalBuffer) {
  const std::string input(
    "\x19\x10\x2a\x00\x00\x00\x54\x2a\x00\x00\x00\x02\x00\x00\x00"
    "\x01\x07\x00\x00\x00\x03\x09\x00\x00\x00\xff"sv);
  const auto bytes = std::as_bytes(std::span(input));
  recording_listener listener;
  ASSERT_TRUE(read_ops(bytes, listener));
  EXPECT_EQ(listener.events().constant_address, bytes.subspan(1).data());
  EXPECT_EQ(listener.events().closure_address, bytes.subspan(6).data());
  EXPECT_EQ(listener.events().captures_address, bytes.subspan(15).data());
  EXPECT_EQ(listener.events().eof_address, bytes.subspan(25).data());
  EXPECT_EQ(listener.events().constants, (std::vector<std::int32_t>{42}));
  ASSERT_EQ(listener.events().captures.size(), 2);
  EXPECT_EQ(std::int32_t{listener.events().captures.front().index}, 7);
  EXPECT_EQ(std::int32_t{listener.events().captures.back().index}, 9);
}

TEST(BytecodeInput, InvalidCaptureKindReportsItsCodeOffset) {
  const auto input = "\x19\x54\x00\x00\x00\x00\x01\x00\x00\x00\x04\x00\x00\x00\x00\xff"sv;
  recording_listener listener;
  const auto result = read_ops(std::as_bytes(std::span(input)), listener);
  ASSERT_FALSE(result);
  EXPECT_EQ(
    result.error(),
    (std::vector<std::string>{"offset 10: code: expected capture kind in [0, 4), but got 4"}));
  EXPECT_TRUE(listener.events().captures.empty());
}

TEST(BytecodeInput, EveryTruncationFailsWithoutEof) {
  const auto complete = "\x54\x00\x00\x00\x00\x01\x00\x00\x00\x02\x03\x00\x00\x00\xff"sv;
  for (std::size_t size = 0; size < complete.size(); ++size) {
    const auto input = std::string_view(complete).substr(0, size);
    recording_listener listener;
    const auto result = read_ops(std::as_bytes(std::span(input)), listener);
    ASSERT_FALSE(result) << size;
    EXPECT_FALSE(listener.events().is_finished) << size;
    ASSERT_EQ(result.error().size(), 1) << size;
    EXPECT_FALSE(result.error().front().empty()) << size;
  }
}

TEST(BytecodeInput, UnknownOpcodeReportsItsCodeOffset) {
  const auto input = "\x00\xff"sv;
  recording_listener listener;
  const auto result = read_ops(std::as_bytes(std::span(input)), listener);
  ASSERT_FALSE(result);
  EXPECT_EQ(result.error(), (std::vector<std::string>{"offset 0: code: unknown opcode 0x00"}));
  EXPECT_FALSE(listener.events().is_finished);
}

TEST(BytecodeInput, InvalidCaptureCountAndKindFail) {
  for (const auto &bytes : {
         "\x54\x00\x00\x00\x00\xff\xff\xff\xff\xff"sv,
         "\x54\x00\x00\x00\x00\x01\x00\x00\x00\x04\x00\x00\x00\x00\xff"sv,
       }) {
    const auto &input = bytes;
    recording_listener listener;
    const auto result = read_ops(std::as_bytes(std::span(input)), listener);
    ASSERT_FALSE(result);
    EXPECT_TRUE(listener.events().captures.empty());
    EXPECT_FALSE(listener.events().is_finished);
    ASSERT_EQ(result.error().size(), 1);
  }
}

TEST(LamacDump, ConstantsExternalCallsAndEof) {
  std::istringstream input(make_file(
    "\x52\x02\x00\x00\x00\x00\x00\x00\x00\x10\x2a\x00\x00\x00\x56\xff"
    "\xff\xff\xff\x01\x00\x00\x00\x16\xff"sv,
    "write\0"sv));
  std::ostringstream output;
  disassembly::legacy listener(output);
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
  disassembly::legacy listener(output);
  ASSERT_TRUE(read_file(input, listener));
  EXPECT_EQ(output.str(), "STRING (\"a\\n\\t\\\"\\\\\\001\")\n");
}

TEST(LamacDump, MinExternalReferenceDoesNotOverflow) {
  std::istringstream input(make_file("\x56\x00\x00\x00\x80\x00\x00\x00\x00\xff"sv));
  std::ostringstream output;
  disassembly::legacy listener(output);
  try {
    EXPECT_TRUE(read_file(input, listener));
    ADD_FAILURE() << "expected lamavm::exception";
  } catch (const lamavm::exception &e) {
    EXPECT_STREQ(e.what(), "invalid string offset 2147483647");
  }
}

TEST(Disassembly, MetadataCallbacksWriteImmediately) {
  const auto table = "name\0"sv;
  std::ostringstream output;
  disassembly::modern listener(output);
  listener(
    header{
      .magic = {'L', 'a', 'M', 'a'},
      .version = 1,
      .string_table_size = static_cast<std::int32_t>(table.size()),
      .globals_count = 0,
      .imports_count = 1,
      .public_symbols_count = 1,
    });
  EXPECT_NE(output.str().find("header:\n"), std::string::npos);

  listener(table);
  EXPECT_NE(output.str().find("\"name\"\n"), std::string::npos);

  listener(import{.name_offset = 0});
  EXPECT_NE(output.str().find("0:\t\"name\" (0)\n"), std::string::npos);

  listener(
    public_symbol{
      .name_offset = 0,
      .code_offset = 0,
      .flag = public_symbol::flag::function,
    });
  EXPECT_NE(output.str().find("0:\tfunction\t\"name\" (0) at 0\n"), std::string::npos);
  EXPECT_EQ(output.str().find("\ncode:\n"), std::string::npos);
}

TEST(Disassembly, PrintsAllFileMetadata) {
  const auto table = "ext\n\0main\0glob\0\0"sv;
  std::string bytes = "LaMa";
  append_int(bytes, 1);
  append_int(bytes, static_cast<std::int32_t>(table.size()));
  append_int(bytes, 2);
  append_int(bytes, 1);
  append_int(bytes, 2);
  bytes += table;
  append_int(bytes, 0);
  constexpr std::int32_t main_name_offset = 5;
  append_int(bytes, main_name_offset);
  append_int(bytes, 0);
  bytes += '\0';
  constexpr std::int32_t global_name_offset = 10;
  append_int(bytes, global_name_offset);
  append_int(bytes, 1);
  bytes += '\1';
  bytes += "\x10\x2a\x00\x00\x00\xff"sv;

  std::istringstream input(bytes);
  std::ostringstream output;
  disassembly::modern listener(output);
  ASSERT_TRUE(read_file(input, listener));

  EXPECT_EQ(
    output.str(),
    "header:\n"
    "  magic:               \t\"LaMa\"\n"
    "  version:             \t1\n"
    "  string_table_size:   \t16\n"
    "  globals_count:       \t2\n"
    "  imports_count:       \t1\n"
    "  public_symbols_count:\t2\n"
    "\n"
    "string_table:\n"
    "  0: \t\"ext\\n\"\n"
    "  5: \t\"main\"\n"
    "  10:\t\"glob\"\n"
    "  15:\t\"\"\n"
    "\n"
    "imports:\n"
    "  0:\t\"ext\\n\" (0)\n"
    "\n"
    "public_symbols:\n"
    "  0:\tfunction\t\"main\" (5) at 0\n"
    "  1:\tglobal  \t\"glob\" (10) at 1\n"
    "\n"
    "code:\n"
    "  0:\t10 2A 00 00 00\tCONST 42\n"
    "  5:\tFF            \tEOF\n");
}

TEST(Disassembly, CodeCallbacksCollectAndCodeSpanFormats) {
  std::ostringstream output;
  disassembly::modern listener(output);
  listener(constant{.value = 1});
  listener(eof{});
  EXPECT_TRUE(output.str().empty());

  const auto code = "\x10\x02\x00\x00\x00\xff"sv;
  ASSERT_TRUE(listener(std::as_bytes(std::span(code))));
  EXPECT_EQ(
    output.str(),
    "  0:\t10 02 00 00 00\tCONST 2\n"
    "  5:\tFF            \tEOF\n");
}

TEST(Disassembly, CodeOffsetsBytesAndOperands) {
  std::istringstream input(make_file(
    "\x52\x02\x00\x00\x00\x03\x00\x00\x00\x10\xff\xff\xff\xff\x01\x56"
    "\xff\xff\xff\xff\x01\x00\x00\x00\x50\x09\x00\x00\x00\x16\xff"sv,
    "write\0"sv));
  std::ostringstream output;
  disassembly::modern listener(output);
  ASSERT_TRUE(read_file(input, listener));
  EXPECT_EQ(
    code_listing(output.str()),
    "  0: \t52 02 00 00 00 03 00 00 00\tBEGIN 2, 3\n"
    "  9: \t10 FF FF FF FF            \tCONST -1\n"
    "  14:\t01                        \tADD\n"
    "  15:\t56 FF FF FF FF 01 00 00 00\tCALL \"write\", 1\n"
    "  24:\t50 09 00 00 00            \tCJMP_Z 9\n"
    "  29:\t16                        \tEND\n"
    "  30:\tFF                        \tEOF\n");
}

TEST(Disassembly, ClosureIncludesCaptureBytesAndKinds) {
  std::istringstream input(make_file(
    "\x54\x2a\x00\x00\x00\x04\x00\x00\x00"
    "\x00\xff\xff\xff\xff\x01\x07\x00\x00\x00"
    "\x02\x03\x00\x00\x00\x03\x09\x00\x00\x00\x19\xff"sv,
    "external\0"sv));
  std::ostringstream output;
  disassembly::modern listener(output);
  ASSERT_TRUE(read_file(input, listener));
  EXPECT_EQ(
    code_listing(output.str()),
    "  0: \t54 2A 00 00 00 04 00 00 00 00 FF FF FF FF 01 07 00 00 00 02 03 00 00 00 03 09 00 00 "
    "00\tCLOSURE 42, [global \"external\", local 7, arg 3, captured 9]\n"
    "  29:\t19                                                                                    "
    "\tDUP\n"
    "  30:\tFF                                                                                    "
    "\tEOF\n");
}

TEST(Disassembly, EscapedStringsStayOnOneLine) {
  std::istringstream input(make_file(
    "\x11\x00\x00\x00\x00\x12\x07\x00\x00\x00\x02\x00\x00\x00"
    "\x57\x07\x00\x00\x00\x02\x00\x00\x00\xff"sv,
    "a\n\t\"\\\x01\0Tag\0"sv));
  std::ostringstream output;
  disassembly::modern listener(output);
  ASSERT_TRUE(read_file(input, listener));
  EXPECT_EQ(
    code_listing(output.str()),
    "  0: \t11 00 00 00 00            \tSTRING \"a\\n\\t\\\"\\\\\\001\"\n"
    "  5: \t12 07 00 00 00 02 00 00 00\tSEXP \"Tag\", 2\n"
    "  14:\t57 07 00 00 00 02 00 00 00\tTAG \"Tag\", 2\n"
    "  23:\tFF                        \tEOF\n");
}

TEST(Disassembly, ListenerResetsCodeOffsetAndStringTable) {
  std::istringstream first(make_file("\x11\x00\x00\x00\x00\xff"sv, "first\0"sv));
  std::istringstream second(make_file("\x11\x00\x00\x00\x00\xff"sv, "second\0"sv));
  std::ostringstream output;
  disassembly::modern listener(output);
  ASSERT_TRUE(read_file(first, listener));
  const auto first_listing = code_listing(output.str());
  output.str("");
  ASSERT_TRUE(read_file(second, listener));
  EXPECT_EQ(
    first_listing + code_listing(output.str()),
    "  0:\t11 00 00 00 00\tSTRING \"first\"\n"
    "  5:\tFF            \tEOF\n"
    "  0:\t11 00 00 00 00\tSTRING \"second\"\n"
    "  5:\tFF            \tEOF\n");
}

TEST(Disassembly, RetainedCodeCanBePrintedAndDecodedAgain) {
  recording_file_listener parsed;
  {
    std::istringstream input(
      make_file("\x10\x2a\x00\x00\x00\x11\x00\x00\x00\x00\xff"sv, "write\0"sv));
    ASSERT_TRUE(read_file(input, parsed));
  }

  const auto &file = parsed.events();
  const auto original_code = file.code;
  std::ostringstream output;
  disassembly::modern printer(output);
  printer(file.file_header);
  printer(file.strings);
  ASSERT_TRUE(printer(file.code));
  EXPECT_EQ(
    code_listing(output.str()),
    "  0: \t10 2A 00 00 00\tCONST 42\n"
    "  5: \t11 00 00 00 00\tSTRING \"write\"\n"
    "  10:\tFF            \tEOF\n");

  EXPECT_EQ(file.code, original_code);

  recording_listener next_consumer;
  ASSERT_TRUE(read_ops(file.code, next_consumer));
  EXPECT_EQ(next_consumer.events().constants, (std::vector<std::int32_t>{42}));
  EXPECT_TRUE(next_consumer.events().is_finished);
}

TEST(Disassembly, InvalidStringReferencesThrow) {
  for (const auto &bytes : {
         make_file("\x56\x00\x00\x00\x80\x00\x00\x00\x00\xff"sv),
         make_file("\x11\xff\xff\xff\xff\xff"sv),
         make_file("\x11\x00\x00\x00\x00\xff"sv, "unterminated"sv),
       }) {
    std::istringstream input(bytes);
    std::ostringstream output;
    disassembly::modern listener(output);
    try {
      EXPECT_TRUE(read_file(input, listener));
      ADD_FAILURE() << "expected lamavm::exception";
    } catch (const lamavm::exception &e) {
      EXPECT_NE(std::string_view(e.what()).find("string"), std::string_view::npos);
    }

    EXPECT_EQ(output.str().find("\tEOF\n"), std::string::npos);
  }
}

} // namespace
} // namespace lamavm::bc
