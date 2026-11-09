#pragma once

#include "layout.hpp"
#include "listener.hpp"

#include <algorithm>
#include <array>
#include <bit>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <format>
#include <ios>
#include <istream>
#include <limits>
#include <span>
#include <string>
#include <string_view>
#include <utility>
#include <variant>
#include <vector>

namespace lamavm::bc {

namespace detail {

using status = std::expected<void, std::monostate>;

template <std::derived_from<base_listener> Listener>
class reader final {
public:
  reader(std::istream &input, Listener &output)
    : in(&input)
    , listener(&output) {
  }

  auto read() -> status {
    enter("header");
    auto h = read_header();
    exit();
    if (!h) {
      return std::unexpected(h.error());
    }

    const auto strings_size = static_cast<std::size_t>(h->string_table_size);
    enter("string table");
    auto strings = read_strings(strings_size);
    exit();
    if (!strings) {
      return std::unexpected(strings.error());
    }

    listener->Listener::operator()(std::string_view(*strings));

    if (auto e = read_imports(*h); !e) {
      return std::unexpected(e.error());
    }

    if (auto e = read_symbols(*h); !e) {
      return std::unexpected(e.error());
    }

    enter("code");
    auto code = read_code();
    exit();
    if (!code) {
      return code;
    }

    if (is_failed) {
      return std::unexpected(std::monostate{});
    }

    return {};
  }

private:
  void enter(std::string_view name) {
    scope = name;
  }

  void exit() {
    scope = {};
  }

  auto error(std::size_t offset, std::string_view message) -> std::unexpected<std::monostate> {
    is_failed = true;
    listener->Listener::operator()(offset, std::format("{}: {}", scope, message));
    return std::unexpected(std::monostate{});
  }

  auto expect(std::string_view name, std::int64_t value, std::int64_t end, std::size_t offset)
    -> status {
    if (value < 0 || value >= end) {
      return error(offset, std::format("expected {} in [0, {}), but got {}", name, end, value));
    }

    return {};
  }

  static auto from_le(std::int32_t value) -> std::int32_t {
    if constexpr (std::endian::native == std::endian::big) {
      return std::byteswap(value);
    }

    return value;
  }

  auto consume(std::size_t count, char *destination) -> status {
    const auto size = static_cast<std::int64_t>(bytes_read) + static_cast<std::int64_t>(count);
    if (auto e = expect("file size", size, max_file_size, bytes_read); !e) {
      return e;
    }

    const auto length = static_cast<std::streamsize>(count);
    if (destination == nullptr) {
      in->ignore(length);
    } else {
      in->read(destination, length);
    }

    const auto actual = static_cast<std::size_t>(in->gcount());
    bytes_read += actual;
    if (actual != count || !*in) {
      return error(bytes_read, std::format("expected {} bytes, but got {}", count, actual));
    }

    return {};
  }

  template <typename T>
  auto read() -> std::expected<T, std::monostate> {
    std::array<char, sizeof(T)> bytes{};
    if (auto e = consume(sizeof(T), bytes.data()); !e) {
      return std::unexpected(e.error());
    }

    return std::bit_cast<T>(bytes);
  }

  auto read_strings(std::size_t size) -> std::expected<std::string, std::monostate> {
    constexpr std::size_t buffer_size = 4096;
    std::array<char, buffer_size> buffer{};
    std::string table;
    while (table.size() < size) {
      const auto count = std::min(size - table.size(), buffer.size());
      if (auto e = consume(count, buffer.data()); !e) {
        return std::unexpected(e.error());
      }

      table.append(buffer.data(), count);
    }

    return table;
  }

  auto read_header() -> std::expected<header, std::monostate> {
    const auto start = bytes_read;
    auto result = read<header>();
    if (!result) {
      return std::unexpected(result.error());
    }

    auto &h = *result;
    h.version = from_le(h.version);
    h.string_table_size = from_le(h.string_table_size);
    h.globals_count = from_le(h.globals_count);
    h.imports_count = from_le(h.imports_count);
    h.public_symbols_count = from_le(h.public_symbols_count);

    if (h.magic != std::array{'L', 'a', 'M', 'a'}) {
      return error(
        start,
        std::format(
          "expected magic 'LaMa', but got '{}'",
          std::string_view(h.magic.data(), h.magic.size())));
    }

    if (h.version != 1) {
      return error(
        start + offsetof(header, version),
        std::format("expected version 1, but got {}", std::int32_t{h.version}));
    }

    constexpr auto end = std::int64_t{std::numeric_limits<std::int32_t>::max()} + 1;
    using field = std::pair<std::string_view, std::int32_t>;
    const std::array fields{
      field{"string_table_size", std::int32_t{h.string_table_size}},
      field{"globals_count", std::int32_t{h.globals_count}},
      field{"imports_count", std::int32_t{h.imports_count}},
      field{"public_symbols_count", std::int32_t{h.public_symbols_count}},
    };
    auto offset = start + offsetof(header, string_table_size);
    for (const auto &[name, value] : fields) {
      if (auto e = expect(name, value, end, offset); !e) {
        return std::unexpected(e.error());
      }

      offset += sizeof(std::int32_t);
    }

    listener->Listener::operator()(std::as_const(h));
    return result;
  }

  auto read_import(const header &h) -> status {
    const auto start = bytes_read;
    auto entry = read<import>();
    if (!entry) {
      return std::unexpected(entry.error());
    }

    entry->name_offset = from_le(entry->name_offset);
    if (const auto e = expect("name_offset", entry->name_offset, h.string_table_size, start); !e) {
      return {};
    }

    listener->Listener::operator()(std::as_const(*entry));
    return {};
  }

  auto read_imports(const header &h) -> status {
    enter("import");
    for (std::int32_t i = 0; i < h.imports_count; ++i) {
      if (auto e = read_import(h); !e) {
        exit();
        return e;
      }
    }

    exit();
    return {};
  }

  auto read_symbol(const header &h) -> status {
    const auto start = bytes_read;
    auto entry = read<public_symbol>();
    if (!entry) {
      return std::unexpected(entry.error());
    }

    entry->name_offset = from_le(entry->name_offset);
    entry->code_offset = from_le(entry->code_offset);
    if (const auto e = expect("name_offset", entry->name_offset, h.string_table_size, start); !e) {
      return {};
    }

    const auto flag = static_cast<std::uint8_t>(entry->flag);
    if (const auto e = expect("flag", flag, 2, start + offsetof(public_symbol, flag)); !e) {
      return {};
    }

    listener->Listener::operator()(std::as_const(*entry));
    return {};
  }

  auto read_symbols(const header &h) -> status {
    enter("symbol");
    for (std::int32_t i = 0; i < h.public_symbols_count; ++i) {
      if (auto e = read_symbol(h); !e) {
        exit();
        return e;
      }
    }

    exit();
    return {};
  }

  template <typename Instruction>
  auto read_instruction() -> status {
    static_assert((sizeof(Instruction) - 1) % sizeof(std::int32_t) == 0);
    std::array<char, sizeof(Instruction)> bytes{};
    bytes.front() = static_cast<char>(Instruction::code);
    if (auto e = consume(bytes.size() - 1, std::span(bytes).subspan(1).data()); !e) {
      return e;
    }

    if constexpr (std::endian::native == std::endian::big) {
      for (std::size_t i = 1; i < bytes.size(); i += sizeof(std::int32_t)) {
        std::ranges::reverse(std::span(bytes).subspan(i, sizeof(std::int32_t)));
      }
    }

    const auto instruction = std::bit_cast<Instruction>(bytes);
    if constexpr (std::same_as<Instruction, closure>) {
      return read_captures(instruction);
    } else {
      listener->Listener::operator()(instruction);
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

    std::vector<closure::capture> captures;
    for (std::int32_t i = 0; i < count; ++i) {
      const auto start = bytes_read;
      auto capture = read<closure::capture>();
      if (!capture) {
        return std::unexpected(capture.error());
      }

      if (auto e = expect("capture kind", static_cast<std::uint8_t>(capture->kind), 4, start); !e) {
        return e;
      }

      capture->index = from_le(capture->index);
      captures.push_back(*capture);
    }

    listener->Listener::operator()(instruction, std::span<const closure::capture>(captures));
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
      listener->Listener::operator()(eof{});
      return {};
    }

    return error(offset, std::format("unknown opcode 0x{:02x}", std::to_underlying(opcode)));
  }

  auto read_code() -> status {
    for (;;) {
      const auto start = bytes_read;
      auto byte = read<bc::opcode>();
      if (!byte) {
        return std::unexpected(byte.error());
      }

      if (auto e = read_opcode(*byte, start); !e) {
        return e;
      }

      if (*byte == eof::code) {
        return {};
      }
    }
  }

  static constexpr std::size_t max_file_size = std::size_t{1024} * 1024 * 1024;

  std::istream *in;
  Listener *listener;
  std::string_view scope;
  std::size_t bytes_read = 0;
  bool is_failed = false;
};

} // namespace detail

template <std::derived_from<base_listener> Listener>
auto read_file(std::istream &in, Listener &listener) -> std::expected<void, std::monostate> {
  return detail::reader<Listener>(in, listener).read();
}

} // namespace lamavm::bc
