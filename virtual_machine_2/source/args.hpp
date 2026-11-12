#pragma once

#include <filesystem>

namespace lamavm {

struct args {
  std::filesystem::path bc_path;

  static args parse(int argc, char* argv[]);
};

} // namespace lamavm
