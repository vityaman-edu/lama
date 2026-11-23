#pragma once

#include "os/mmap.hpp"

namespace lamavm::interpreter {

[[nodiscard]] os::mmap map_stack();

} // namespace lamavm::interpreter
