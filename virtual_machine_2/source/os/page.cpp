#include "page.hpp"

#include "core/assert.hpp"

#include <cstddef>

#include <unistd.h>

namespace lamavm::os {

std::size_t page_size() {
  static thread_local const auto size = [] {
    const auto value = ::sysconf(_SC_PAGESIZE);
    LAMAVM_EXPECT_B(0, <, value);
    return static_cast<std::size_t>(value);
  }();

  return size;
}

} // namespace lamavm::os
