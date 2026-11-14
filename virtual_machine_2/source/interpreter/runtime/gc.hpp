#pragma once

namespace lamavm::interpreter {

class gc {
public:
  gc() = delete;

  static void init();

  static void set_bot(void* x);

  static void set_top(void* x);
};

} // namespace lamavm::interpreter
