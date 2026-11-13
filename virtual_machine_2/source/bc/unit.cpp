#include "unit.hpp"

#include "bc/limit.hpp"
#include "layout.hpp"

#include "core/assert.hpp"

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <ios>
#include <istream>
#include <span>
#include <string_view>
#include <utility>
#include <vector>

namespace lamavm::bc {

using namespace std::string_view_literals;

const header_t& unit::header() const {
  return *reinterpret_cast<const header_t*>(arena.data());
}

std::span<const char> unit::string_table() const {
  return {
    reinterpret_cast<const char*>(arena.data() + sizeof(header_t)),
    static_cast<std::size_t>(header().string_table_size),
  };
}

std::span<const import_t> unit::imports() const {
  const auto offset = sizeof(header_t) + string_table().size_bytes();
  return {
    reinterpret_cast<const import_t*>(arena.data() + offset),
    static_cast<std::size_t>(header().imports_count),
  };
}

std::span<const public_symbol_t> unit::public_symbols() const {
  const auto offset = sizeof(header_t) + string_table().size_bytes() + imports().size_bytes();
  return {
    reinterpret_cast<const public_symbol_t*>(arena.data() + offset),
    static_cast<std::size_t>(header().public_symbols_count),
  };
}

std::span<const std::uint8_t> unit::code_section() const {
  const auto offset = sizeof(header_t) +
                      string_table().size_bytes() +
                      imports().size_bytes() +
                      public_symbols().size_bytes();
  return {
    reinterpret_cast<const std::uint8_t*>(arena.data() + offset),
    arena.size() - offset,
  };
}

unit unit::parse(std::vector<std::byte> arena) {
  unit u(std::move(arena));

  LAMAVM_EXPECT_B(sizeof(header_t), <=, u.arena.size());
  const auto& header = u.header();
  const std::string_view header_magic(header.magic.data(), header.magic.size());
  LAMAVM_EXPECT_B(header_magic, ==, "LaMa"sv);
  LAMAVM_EXPECT_B(header.version, ==, 1);
  LAMAVM_EXPECT_B(0, <=, header.string_table_size);
  LAMAVM_EXPECT_BB(0, <=, header.globals_count, <=, max_globals_count);
  LAMAVM_EXPECT_BB(0, <=, header.imports_count, <=, max_imports_count);
  LAMAVM_EXPECT_BB(0, <=, header.public_symbols_count, <=, max_public_symbols_count);

  const auto code_section = u.code_section();
  const auto code_offset  = reinterpret_cast<const std::byte*>(code_section.data()) -
                           reinterpret_cast<const std::byte*>(u.arena.data());
  const auto arena_size = static_cast<std::ptrdiff_t>(u.arena.size());
  LAMAVM_EXPECT_B(code_offset, <, arena_size);

  const auto string_table_size = static_cast<std::int32_t>(u.string_table().size());
  const auto code_section_size = static_cast<std::int32_t>(code_section.size());

  for (const import_t& i : u.imports()) {
    LAMAVM_EXPECT_BB(0, <=, i.name_offset, <, string_table_size);
  }

  for (const public_symbol_t& s : u.public_symbols()) {
    LAMAVM_EXPECT_BB(0, <=, s.name_offset, <=, string_table_size);
    LAMAVM_EXPECT_BB(0, <=, s.code_offset, <=, code_section_size);
    LAMAVM_EXPECT_BB(0, <=, s.flag, <=, 1);
  }

  LAMAVM_EXPECT_B(0U, <, code_section.size());
  LAMAVM_EXPECT_B(code_section.back(), ==, std::uint8_t{0xFF});

  return u;
}

unit unit::parse(const std::filesystem::path& path) {
  LAMAVM_EXPECT_B(path.extension().string(), ==, ".bc"sv);

  std::ifstream f;
  f.exceptions(std::ifstream::failbit | std::ifstream::badbit);
  f.open(path, std::ifstream::binary | std::ifstream::ate);

  const auto size = static_cast<std::size_t>(f.tellg());
  LAMAVM_EXPECT_B(size, <=, max_unit_size);

  std::vector<std::byte> arena(size);
  f.seekg(0);
  f.read(reinterpret_cast<char*>(arena.data()), static_cast<std::streamsize>(arena.size()));
  return parse(std::move(arena));
}

unit::unit(std::vector<std::byte> arena)
  : arena(std::move(arena)) {
}

} // namespace lamavm::bc
