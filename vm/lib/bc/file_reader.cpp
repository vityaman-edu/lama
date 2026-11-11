#include "file_reader.hpp"

#include "core/exception.hpp"
#include "file_listener.hpp"
#include "layout.hpp"

#include <algorithm>
#include <array>
#include <bit>
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
namespace {

using parse_status = std::expected<void, std::monostate>;

class file_reader final {
public:
  file_reader(std::istream &input, file_listener &output)
    : in(&input)
    , listener(&output) {
  }

  auto read() -> std::expected<void, std::vector<error>> {
    auto h =
      scoped("header", [this] -> std::expected<header, std::monostate> { return read_header(); });
    if (!h) {
      return std::unexpected(std::move(errors));
    }

    const auto strings_size = static_cast<std::size_t>(h->string_table_size);
    auto strings =
      scoped("string table", [this, strings_size] -> std::expected<std::string, std::monostate> {
        return read_strings(strings_size);
      });
    if (!strings) {
      return std::unexpected(std::move(errors));
    }

    const auto last_terminator = strings->rfind('\0');
    string_table_end = last_terminator == std::string::npos ? 0 : last_terminator + 1;

    if (auto result = listener->operator()(*strings); !result) {
      errors.push_back(std::move(result.error()));
      return std::unexpected(std::move(errors));
    }

    if (
      const auto e = scoped("import", [this, &h] -> parse_status { return read_imports(*h); });
      !e) {
      return std::unexpected(std::move(errors));
    }

    if (
      const auto e = scoped("symbol", [this, &h] -> parse_status { return read_symbols(*h); });
      !e) {
      return std::unexpected(std::move(errors));
    }

    auto code = scoped("code", [this] -> std::expected<std::vector<std::byte>, std::monostate> {
      return read_code_bytes();
    });
    if (!code) {
      return std::unexpected(std::move(errors));
    }

    if (!errors.empty()) {
      return std::unexpected(std::move(errors));
    }

    return listener->operator()(*code);
  }

private:
  auto notify(const auto &op) -> parse_status {
    if (auto result = listener->operator()(op); !result) {
      errors.push_back(std::move(result.error()));
      return std::unexpected(std::monostate{});
    }

    return {};
  }

  auto scoped(std::string_view name, auto f) -> decltype(f()) {
    const auto previous = std::exchange(scope, name);
    auto result = f();
    scope = previous;
    return result;
  }

  auto make_error(std::size_t offset, std::string_view message) -> std::unexpected<std::monostate> {
    errors.push_back(std::format("offset {}: {}: {}", offset, scope, message));
    return std::unexpected(std::monostate{});
  }

  auto expect(std::string_view name, std::int64_t value, std::int64_t end, std::size_t offset)
    -> parse_status {
    if (value < 0 || value >= end) {
      return make_error(
        offset,
        std::format("expected {} in [0, {}), but got {}", name, end, value));
    }

    return {};
  }

  auto consume(std::size_t count, char *destination) -> parse_status {
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
      return make_error(bytes_read, std::format("expected {} bytes, but got {}", count, actual));
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
    table.reserve(std::min(size, buffer_size));
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

    if (h.magic != std::array{'L', 'a', 'M', 'a'}) {
      return make_error(
        start,
        std::format(
          "expected magic 'LaMa', but got '{}'",
          std::string_view(h.magic.data(), h.magic.size())));
    }

    if (h.version != 1) {
      return make_error(
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

    if (auto e = notify(std::as_const(h)); !e) {
      return std::unexpected(e.error());
    }

    return result;
  }

  auto validate_name(std::int32_t value, const header &h, std::size_t offset) -> parse_status {
    if (const auto e = expect("name_offset", value, h.string_table_size, offset); !e) {
      return e;
    }

    if (std::cmp_greater_equal(value, string_table_end)) {
      return make_error(offset, std::format("unterminated string at offset {}", value));
    }

    return {};
  }

  auto read_import(const header &h) -> parse_status {
    const auto start = bytes_read;

    auto entry = read<import>();
    if (!entry) {
      return std::unexpected(entry.error());
    }

    if (const auto e = validate_name(entry->name_offset, h, start); !e) {
      return {};
    }

    return notify(std::as_const(*entry));
  }

  auto read_imports(const header &h) -> parse_status {
    for (std::int32_t i = 0; i < h.imports_count; ++i) {
      if (auto e = read_import(h); !e) {
        return e;
      }
    }

    return {};
  }

  auto read_symbol(const header &h) -> parse_status {
    const auto start = bytes_read;

    auto entry = read<public_symbol>();
    if (!entry) {
      return std::unexpected(entry.error());
    }

    if (const auto e = validate_name(entry->name_offset, h, start); !e) {
      return {};
    }

    const auto flag = static_cast<std::uint8_t>(entry->flag);
    if (const auto e = expect("flag", flag, 2, start + offsetof(public_symbol, flag)); !e) {
      return {};
    }

    return notify(std::as_const(*entry));
  }

  auto read_symbols(const header &h) -> parse_status {
    for (std::int32_t i = 0; i < h.public_symbols_count; ++i) {
      if (auto e = read_symbol(h); !e) {
        return e;
      }
    }

    return {};
  }

  auto read_code_bytes() -> std::expected<std::vector<std::byte>, std::monostate> {
    constexpr std::size_t buffer_size = 4096;
    std::array<char, buffer_size> buffer{};
    std::vector<std::byte> code;
    for (;;) {
      in->read(buffer.data(), static_cast<std::streamsize>(buffer.size()));
      const auto actual = static_cast<std::size_t>(in->gcount());
      bytes_read += actual;
      if (bytes_read >= max_file_size) {
        return make_error(bytes_read - actual, "file size exceeds the limit");
      }

      const auto bytes = std::as_bytes(std::span(buffer).first(actual));
      code.insert(code.end(), bytes.begin(), bytes.end());
      if (in->bad() || (in->fail() && !in->eof())) {
        return make_error(bytes_read, "failed to read code bytes");
      }

      if (in->eof()) {
        return code;
      }
    }
  }

  static constexpr std::size_t max_file_size = std::size_t{1024} * 1024 * 1024;

  std::istream *in;
  file_listener *listener;
  std::string_view scope;
  std::size_t bytes_read = 0;
  std::size_t string_table_end = 0;
  std::vector<error> errors;
};

} // namespace

auto read_file(std::istream &in, file_listener &listener)
  -> std::expected<void, std::vector<error>> {
  return file_reader(in, listener).read();
}

} // namespace lamavm::bc
