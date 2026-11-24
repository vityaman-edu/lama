#pragma once

#include <cstddef>

namespace lamavm::interpreter {

class gc {
public:
  gc() = delete;

  static void init();

  static std::size_t& bot();

  static std::size_t& top();
};

} // namespace lamavm::interpreter
