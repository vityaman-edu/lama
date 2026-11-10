#include "code_reader.hpp"

#include "code_listener.hpp"
#include "layout.hpp"

#include <algorithm>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <format>
#include <span>
#include <string_view>
#include <type_traits>
#include <utility>
#include <variant>

namespace lamavm::bc {
namespace {

using status = std::expected<void, std::monostate>;

class code_reader final {
public:
  code_reader(std::span<const std::byte> input, code_listener &output)
    : in(input)
    , listener(&output) {
  }

  auto read() -> status {
    if (in.size() >= max_file_size) {
      return error(0, "code size exceeds the limit");
    }

    return read_instructions();
  }

private:
  auto error(std::size_t offset, std::string_view message) -> std::unexpected<std::monostate> {
    listener->operator()(offset, std::format("code: {}", message));
    return std::unexpected(std::monostate{});
  }

  auto expect(std::string_view name, std::int64_t value, std::int64_t end, std::size_t offset)
    -> status {
    if (value < 0 || value >= end) {
      return error(offset, std::format("expected {} in [0, {}), but got {}", name, end, value));
    }

    return {};
  }

  auto consume(std::size_t count) -> std::expected<std::span<const std::byte>, std::monostate> {
    const auto actual = std::min(count, in.size() - bytes_read);
    const auto bytes = in.subspan(bytes_read, actual);
    bytes_read += actual;
    if (actual != count) {
      return error(bytes_read, std::format("expected {} bytes, but got {}", count, actual));
    }

    return bytes;
  }

  template <typename T>
  auto read(std::size_t count) -> std::expected<std::span<const T>, std::monostate> {
    static_assert(alignof(T) == 1);
    static_assert(std::is_trivially_copyable_v<T>);

    auto bytes = consume(count * sizeof(T));
    if (!bytes) {
      return std::unexpected(bytes.error());
    }

    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    return std::span<const T>(reinterpret_cast<const T *>(bytes->data()), count);
  }

  template <typename Instruction>
  auto read_instruction() -> status {
    auto instructions = read<Instruction>(1);
    if (!instructions) {
      return std::unexpected(instructions.error());
    }

    const auto &instruction = instructions->front();
    if constexpr (std::same_as<Instruction, closure>) {
      return read_captures(instruction);
    } else {
      listener->operator()(instruction);
      return {};
    }
  }

  auto read_captures(const closure &instruction) -> status {
    const auto count = instruction.n_captured;
    if (
      auto e = expect(
        "n_captured",
        count,
        max_file_size / sizeof(closure::capture),
        bytes_read - sizeof(std::int32_t));
      !e) {
      return e;
    }

    auto offset = bytes_read;
    auto captures = read<closure::capture>(static_cast<std::size_t>(count));
    if (!captures) {
      return std::unexpected(captures.error());
    }

    for (const auto &capture : *captures) {
      if (auto e = expect("capture kind", static_cast<std::uint8_t>(capture.kind), 4, offset); !e) {
        return e;
      }

      offset += sizeof(closure::capture);
    }

    listener->operator()(instruction, *captures);
    return {};
  }

  auto read_opcode(bc::opcode opcode, std::size_t offset) -> status {
    switch (opcode) {
    case add::code:
      return read_instruction<add>();
    case sub::code:
      return read_instruction<sub>();
    case mul::code:
      return read_instruction<mul>();
    case div::code:
      return read_instruction<div>();
    case mod::code:
      return read_instruction<mod>();
    case lt::code:
      return read_instruction<lt>();
    case le::code:
      return read_instruction<le>();
    case gt::code:
      return read_instruction<gt>();
    case ge::code:
      return read_instruction<ge>();
    case eq::code:
      return read_instruction<eq>();
    case ne::code:
      return read_instruction<ne>();
    case logical_and::code:
      return read_instruction<logical_and>();
    case logical_or::code:
      return read_instruction<logical_or>();
    case constant::code:
      return read_instruction<constant>();
    case string::code:
      return read_instruction<string>();
    case sexp::code:
      return read_instruction<sexp>();
    case sti::code:
      return read_instruction<sti>();
    case sta::code:
      return read_instruction<sta>();
    case jmp::code:
      return read_instruction<jmp>();
    case end::code:
      return read_instruction<end>();
    case ret::code:
      return read_instruction<ret>();
    case drop::code:
      return read_instruction<drop>();
    case dup::code:
      return read_instruction<dup>();
    case swap::code:
      return read_instruction<swap>();
    case elem::code:
      return read_instruction<elem>();
    case ld_glo::code:
      return read_instruction<ld_glo>();
    case ld_loc::code:
      return read_instruction<ld_loc>();
    case ld_arg::code:
      return read_instruction<ld_arg>();
    case ld_clo::code:
      return read_instruction<ld_clo>();
    case lda_glo::code:
      return read_instruction<lda_glo>();
    case lda_loc::code:
      return read_instruction<lda_loc>();
    case lda_arg::code:
      return read_instruction<lda_arg>();
    case lda_clo::code:
      return read_instruction<lda_clo>();
    case st_glo::code:
      return read_instruction<st_glo>();
    case st_loc::code:
      return read_instruction<st_loc>();
    case st_arg::code:
      return read_instruction<st_arg>();
    case st_clo::code:
      return read_instruction<st_clo>();
    case cjmp_z::code:
      return read_instruction<cjmp_z>();
    case cjmp_nz::code:
      return read_instruction<cjmp_nz>();
    case begin::code:
      return read_instruction<begin>();
    case begin_closure::code:
      return read_instruction<begin_closure>();
    case closure::code:
      return read_instruction<closure>();
    case callc::code:
      return read_instruction<callc>();
    case call::code:
      return read_instruction<call>();
    case tag::code:
      return read_instruction<tag>();
    case array::code:
      return read_instruction<array>();
    case fail::code:
      return read_instruction<fail>();
    case fail_keep::code:
      return read_instruction<fail_keep>();
    case line::code:
      return read_instruction<line>();
    case patt_str_cmp::code:
      return read_instruction<patt_str_cmp>();
    case patt_string::code:
      return read_instruction<patt_string>();
    case patt_array::code:
      return read_instruction<patt_array>();
    case patt_sexp::code:
      return read_instruction<patt_sexp>();
    case patt_boxed::code:
      return read_instruction<patt_boxed>();
    case patt_unboxed::code:
      return read_instruction<patt_unboxed>();
    case patt_closure::code:
      return read_instruction<patt_closure>();
    case barray::code:
      return read_instruction<barray>();
    case eof::code:
      return read_instruction<eof>();
    }

    return error(offset, std::format("unknown opcode 0x{:02x}", std::to_underlying(opcode)));
  }

  auto read_instructions() -> status {
    for (;;) {
      const auto start = bytes_read;
      if (bytes_read == in.size()) {
        return error(bytes_read, "expected 1 bytes, but got 0");
      }

      const auto byte = static_cast<bc::opcode>(in.subspan(bytes_read).front());
      if (auto e = read_opcode(byte, start); !e) {
        return e;
      }

      if (byte == eof::code) {
        return {};
      }
    }
  }

  static constexpr std::size_t max_file_size = std::size_t{1024} * 1024 * 1024;

  std::span<const std::byte> in;
  code_listener *listener;
  std::size_t bytes_read = 0;
};

} // namespace

auto read_code(std::span<const std::byte> code, code_listener &listener)
  -> std::expected<void, std::monostate> {
  return code_reader(code, listener).read();
}

} // namespace lamavm::bc
