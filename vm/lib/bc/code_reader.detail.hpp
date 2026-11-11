#pragma once

#include "core/exception.hpp"
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

namespace lamavm::bc::detail {

template <typename Listener, bool unsafe>
class code_reader final {
public:
  code_reader(std::span<const std::byte> input, Listener &output)
    : in(input)
    , listener(&output) {
  }

  auto read_op() -> status {
    if constexpr (!unsafe) {
      if (in.size() >= max_file_size) {
        return make_error(0, "code size exceeds the limit");
      }

      if (bytes_read == in.size()) {
        return make_error(bytes_read, "expected 1 bytes, but got 0");
      }
    }

    const auto byte = static_cast<bc::opcode>(in.subspan(bytes_read).front());
    return read_opcode(byte, bytes_read);
  }

  auto read_ops() -> status {
    for (;;) {
      const auto start = bytes_read;
      if (auto e = read_op(); !e) {
        return e;
      }

      if (static_cast<bc::opcode>(in.subspan(start).front()) == eof::code) {
        return {};
      }
    }
  }

private:
  static auto make_error(std::size_t offset, std::string_view message) -> std::unexpected<error> {
    return std::unexpected(std::format("offset {}: code: {}", offset, message));
  }

  static auto
  expect(std::string_view name, std::int64_t value, std::int64_t end, std::size_t offset)
    -> status {
    if (value < 0 || value >= end) {
      return make_error(
        offset,
        std::format("expected {} in [0, {}), but got {}", name, end, value));
    }

    return {};
  }

  auto consume(std::size_t count) -> std::expected<std::span<const std::byte>, error> {
    const auto actual = unsafe ? count : std::min(count, in.size() - bytes_read);
    const auto bytes = in.subspan(bytes_read, actual);
    bytes_read += actual;
    if constexpr (!unsafe) {
      if (actual != count) {
        return make_error(bytes_read, std::format("expected {} bytes, but got {}", count, actual));
      }
    }

    return bytes;
  }

  template <typename T>
  auto read(std::size_t count) -> std::expected<std::span<const T>, error> {
    static_assert(alignof(T) == 1);
    static_assert(std::is_trivially_copyable_v<T>);

    auto bytes = consume(count * sizeof(T));
    if constexpr (!unsafe) {
      if (!bytes) {
        return std::unexpected(bytes.error());
      }
    }

    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    return std::span<const T>(reinterpret_cast<const T *>(bytes->data()), count);
  }

  template <typename Op>
  auto read_op() -> status {
    auto ops = read<Op>(1);
    if constexpr (!unsafe) {
      if (!ops) {
        return std::unexpected(ops.error());
      }
    }

    const auto &op = ops->front();
    if constexpr (std::same_as<Op, closure>) {
      return read_captures(op);
    } else {
      return listener->operator()(op);
    }
  }

  auto read_captures(const closure &op) -> status {
    const auto count = op.n_captured;

    if constexpr (!unsafe) {
      if (
        auto e = expect(
          "n_captured",
          count,
          max_file_size / sizeof(closure::capture),
          bytes_read - sizeof(std::int32_t));
        !e) {
        return e;
      }
    }

    auto captures = read<closure::capture>(static_cast<std::size_t>(count));

    if constexpr (!unsafe) {
      if (!captures) {
        return std::unexpected(captures.error());
      }

      auto offset = bytes_read - captures->size_bytes();
      for (const auto &capture : *captures) {
        if (
          auto e = expect("capture kind", static_cast<std::uint8_t>(capture.kind), 4, offset); !e) {
          return e;
        }

        offset += sizeof(closure::capture);
      }
    }

    return listener->operator()(op, *captures);
  }

  auto read_opcode(bc::opcode opcode, std::size_t offset) -> status {
    switch (opcode) {
    case add::code:
      return read_op<add>();
    case sub::code:
      return read_op<sub>();
    case mul::code:
      return read_op<mul>();
    case div::code:
      return read_op<div>();
    case mod::code:
      return read_op<mod>();
    case lt::code:
      return read_op<lt>();
    case le::code:
      return read_op<le>();
    case gt::code:
      return read_op<gt>();
    case ge::code:
      return read_op<ge>();
    case eq::code:
      return read_op<eq>();
    case ne::code:
      return read_op<ne>();
    case logical_and::code:
      return read_op<logical_and>();
    case logical_or::code:
      return read_op<logical_or>();
    case constant::code:
      return read_op<constant>();
    case string::code:
      return read_op<string>();
    case sexp::code:
      return read_op<sexp>();
    case sti::code:
      return read_op<sti>();
    case sta::code:
      return read_op<sta>();
    case jmp::code:
      return read_op<jmp>();
    case end::code:
      return read_op<end>();
    case ret::code:
      return read_op<ret>();
    case drop::code:
      return read_op<drop>();
    case dup::code:
      return read_op<dup>();
    case swap::code:
      return read_op<swap>();
    case elem::code:
      return read_op<elem>();
    case ld_glo::code:
      return read_op<ld_glo>();
    case ld_loc::code:
      return read_op<ld_loc>();
    case ld_arg::code:
      return read_op<ld_arg>();
    case ld_clo::code:
      return read_op<ld_clo>();
    case lda_glo::code:
      return read_op<lda_glo>();
    case lda_loc::code:
      return read_op<lda_loc>();
    case lda_arg::code:
      return read_op<lda_arg>();
    case lda_clo::code:
      return read_op<lda_clo>();
    case st_glo::code:
      return read_op<st_glo>();
    case st_loc::code:
      return read_op<st_loc>();
    case st_arg::code:
      return read_op<st_arg>();
    case st_clo::code:
      return read_op<st_clo>();
    case cjmp_z::code:
      return read_op<cjmp_z>();
    case cjmp_nz::code:
      return read_op<cjmp_nz>();
    case begin::code:
      return read_op<begin>();
    case begin_closure::code:
      return read_op<begin_closure>();
    case closure::code:
      return read_op<closure>();
    case callc::code:
      return read_op<callc>();
    case call::code:
      return read_op<call>();
    case tag::code:
      return read_op<tag>();
    case array::code:
      return read_op<array>();
    case fail::code:
      return read_op<fail>();
    case fail_keep::code:
      return read_op<fail_keep>();
    case line::code:
      return read_op<line>();
    case patt_str_cmp::code:
      return read_op<patt_str_cmp>();
    case patt_string::code:
      return read_op<patt_string>();
    case patt_array::code:
      return read_op<patt_array>();
    case patt_sexp::code:
      return read_op<patt_sexp>();
    case patt_boxed::code:
      return read_op<patt_boxed>();
    case patt_unboxed::code:
      return read_op<patt_unboxed>();
    case patt_closure::code:
      return read_op<patt_closure>();
    case barray::code:
      return read_op<barray>();
    case eof::code:
      return read_op<eof>();
    }

    if constexpr (unsafe) {
      std::unreachable();
    } else {
      return make_error(offset, std::format("unknown opcode 0x{:02x}", std::to_underlying(opcode)));
    }
  }

  static constexpr std::size_t max_file_size = std::size_t{1024} * 1024 * 1024;

  std::span<const std::byte> in;
  Listener *listener;
  std::size_t bytes_read = 0;
};

} // namespace lamavm::bc::detail
