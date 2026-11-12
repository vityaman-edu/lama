#include "unit.hpp"

#include "bc/layout.hpp"
#include "core/assert.hpp"

#include <cstddef>
#include <filesystem>
#include <fstream>
#include <istream>
#include <iterator>
#include <span>
#include <string_view>
#include <utility>
#include <vector>

namespace lamavm::bc {

unit_t parse_unit(std::vector<char> buffer) {
  unit_t u = {
    .buffer = std::move(buffer),
    .string_table = "",
    .code = {},
  };

  const char* data = u.buffer.data();

  const auto* h = reinterpret_cast<const header_t*>(data);

  LAMAVM_ASSERT_EQ(std::string_view{h->magic}, "LaMa");
  LAMAVM_ASSERT_EQ(h->version, 1);
  LAMAVM_ASSERT_LE(0, h->string_table_size);
  LAMAVM_ASSERT_LE(0, h->globals_count);
  LAMAVM_ASSERT_LE(0, h->imports_count);
  LAMAVM_ASSERT_LE(0, h->public_symbols_count);

  const size_t off_string_table = sizeof(header_t);
  const size_t off_imports = off_string_table + (sizeof(char) * h->string_table_size);
  const size_t off_public_symbols = off_imports + (sizeof(import_t) * h->imports_count);
  const size_t off_code = off_public_symbols + (sizeof(public_symbol_t) * h->public_symbols_count);

  LAMAVM_ASSERT_LT(off_code, u.buffer.size());

  u.string_table = {
    data + off_string_table,
    static_cast<size_t>(h->string_table_size),
  };

  LAMAVM_ASSERT_EQ(0, h->globals_count);

  std::span<const import_t> const imports = {
    reinterpret_cast<const import_t*>(data + off_imports),
    static_cast<size_t>(h->imports_count),
  };

  LAMAVM_ASSERT_EQ(imports.size(), 1);
  LAMAVM_ASSERT_EQ(u.string_table.substr(imports.front().name_offset), "Std");

  LAMAVM_ASSERT_EQ(0, h->public_symbols_count);

  u.code = {
    reinterpret_cast<const std::uint8_t*>(data + off_code),
    reinterpret_cast<const std::uint8_t*>(data + u.buffer.size()),
  };

  LAMAVM_ASSERT_LT(0, u.code.size());
  LAMAVM_ASSERT_EQ(u.code.back(), std::uint8_t{0xFF});

  return u;
}

unit_t parse_unit(std::istream& in) {
  using it = std::istreambuf_iterator<char>;

  in.exceptions(std::ifstream::failbit | std::ifstream::badbit);
  std::vector<char> buffer(it{in}, it{});
  return parse_unit(std::move(buffer));
}

unit_t parse_unit(const std::filesystem::path& path) {
  LAMAVM_ASSERT_EQ(path.extension(), ".bc");

  std::ifstream f(path);
  return parse_unit(f);
}

} // namespace lamavm::bc
