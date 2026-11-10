#pragma once

#include <expected>

namespace lamavm {

struct args {
  bool ds = false;

  static auto parse(int argc, char **argv) -> std::expected<args, int>;
};

} // namespace lamavm
