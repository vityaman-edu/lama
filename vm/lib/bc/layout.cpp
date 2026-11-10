#include "layout.hpp"

#include <bit>

static_assert(std::endian::native == std::endian::little, "bytecode requires a little-endian host");
