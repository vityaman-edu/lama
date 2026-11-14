#pragma once

#include <cstdint>

namespace lamavm::op {

// Plantime handler representation.
// At parsing, it is an opcode_t value.
using handler_id_t = std::uintptr_t;

// Runtime handler representation.
// NB. Implementation ends with [gnu::musttail]] call.
using handler_fn_t = void (*)();

static_assert(sizeof(handler_id_t) == sizeof(handler_fn_t));

} // namespace lamavm::op
