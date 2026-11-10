#include "file_reader.hpp"

#include "code_listener_nop.hpp"
#include "code_reader.hpp"
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

using status = std::expected<void, std::monostate>;

class file_reader final {
public:
  file_reader(std::istream &input, file_listener &output)
    : in(&input)
    , listener(&output) {
  }

  auto read() -> std::expected<void, std::vector<std::string>> {
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

    listener->operator()(std::string_view(*strings));

    if (const auto e = scoped("import", [this, &h] -> status { return read_imports(*h); }); !e) {
      return std::unexpected(std::move(errors));
    }

    if (const auto e = scoped("symbol", [this, &h] -> status { return read_symbols(*h); }); !e) {
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

    const auto bytes = std::span<const std::byte>(*code);
    code_listener_nop validator;
    if (auto e = read_ops(bytes, validator); !e) {
      return e;
    }

    return listener->operator()(bytes);
  }

private:
  auto scoped(std::string_view name, auto f) -> decltype(f()) {
    const auto previous = std::exchange(scope, name);
    auto result = f();
    scope = previous;
    return result;
  }

  auto error(std::size_t offset, std::string_view message) -> std::unexpected<std::monostate> {
    errors.push_back(std::format("offset {}: {}: {}", offset, scope, message));
    return std::unexpected(std::monostate{});
  }

  auto expect(std::string_view name, std::int64_t value, std::int64_t end, std::size_t offset)
    -> status {
    if (value < 0 || value >= end) {
      return error(offset, std::format("expected {} in [0, {}), but got {}", name, end, value));
    }

    return {};
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

    listener->operator()(std::as_const(h));
    return result;
  }

  auto read_import(const header &h) -> status {
    const auto start = bytes_read;

    auto entry = read<import>();
    if (!entry) {
      return std::unexpected(entry.error());
    }

    if (const auto e = expect("name_offset", entry->name_offset, h.string_table_size, start); !e) {
      return {};
    }

    listener->operator()(std::as_const(*entry));
    return {};
  }

  auto read_imports(const header &h) -> status {
    for (std::int32_t i = 0; i < h.imports_count; ++i) {
      if (auto e = read_import(h); !e) {
        return e;
      }
    }

    return {};
  }

  auto read_symbol(const header &h) -> status {
    const auto start = bytes_read;

    auto entry = read<public_symbol>();
    if (!entry) {
      return std::unexpected(entry.error());
    }

    if (const auto e = expect("name_offset", entry->name_offset, h.string_table_size, start); !e) {
      return {};
    }

    const auto flag = static_cast<std::uint8_t>(entry->flag);
    if (const auto e = expect("flag", flag, 2, start + offsetof(public_symbol, flag)); !e) {
      return {};
    }

    listener->operator()(std::as_const(*entry));
    return {};
  }

  auto read_symbols(const header &h) -> status {
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
        return error(bytes_read - actual, "file size exceeds the limit");
      }

      const auto bytes = std::as_bytes(std::span(buffer).first(actual));
      code.insert(code.end(), bytes.begin(), bytes.end());
      if (in->bad() || (in->fail() && !in->eof())) {
        return error(bytes_read, "failed to read code bytes");
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
  std::vector<std::string> errors;
};

} // namespace

auto read_file(std::istream &in, file_listener &listener)
  -> std::expected<void, std::vector<std::string>> {
  return file_reader(in, listener).read();
}

} // namespace lamavm::bc
