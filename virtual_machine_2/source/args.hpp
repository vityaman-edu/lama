#pragma once

#include <filesystem>

namespace lamavm {

struct args {
  std::filesystem::path bc_path;

  // NOLINTNEXTLINE(modernize-avoid-c-arrays)
  static args parse(int argc, char* argv[]);
};

} // namespace lamavm
