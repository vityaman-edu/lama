#pragma once

#include <cstdint>
#include <expected>
#include <string>

namespace lamavm {

enum class disassembly_kind : std::uint8_t {
  none,
  legacy,
  modern,
};

struct args {
  bool is_help = false;
  disassembly_kind disassembly = disassembly_kind::none;

  static auto parse(int argc, char **argv) -> std::expected<args, std::string>;
};

} // namespace lamavm
