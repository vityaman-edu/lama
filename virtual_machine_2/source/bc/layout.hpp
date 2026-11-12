#pragma once

#include <array>
#include <cstdint>

namespace lamavm::bc {

struct [[gnu::packed]] header_t {
  std::array<char, 4> magic = {0};
  std::int32_t version = 0;
  std::int32_t string_table_size = 0;
  std::int32_t globals_count = 0;
  std::int32_t imports_count = 0;
  std::int32_t public_symbols_count = 0;
};

struct [[gnu::packed]] import_t {
  std::int32_t name_offset = 0;
};

struct [[gnu::packed]] public_symbol_t {
  std::int32_t name_offset = 0;
  std::int32_t code_offset = 0;
  bool flag = false;
};

}
