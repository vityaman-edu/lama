#pragma once

#include "bc/layout.hpp"

#include <cstddef>
#include <filesystem>
#include <span>
#include <vector>

namespace lamavm::bc {

class unit final {
public:
  [[nodiscard]] const header_t& header() const;
  [[nodiscard]] std::span<const char> string_table() const;
  [[nodiscard]] std::span<const import_t> imports() const;
  [[nodiscard]] std::span<const public_symbol_t> public_symbols() const;
  [[nodiscard]] std::span<const std::uint8_t> code_section() const;

  [[nodiscard]] std::size_t size() const;
  [[nodiscard]] const char* string(std::size_t offset) const;

  static unit parse(std::vector<std::byte> arena);
  static unit parse(const std::filesystem::path& path);

private:
  explicit unit(std::vector<std::byte> arena);

  std::vector<std::byte> arena;
};

} // namespace lamavm::bc
