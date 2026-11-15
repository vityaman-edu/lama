#pragma once

#include "op/word.hpp"

#include <span>

namespace lamavm::interpreter {

void decode(std::span<op::word_t> text);

} // namespace lamavm::interpreter
