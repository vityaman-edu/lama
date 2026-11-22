#pragma once

#include "box.hpp"

#include "core/assert.hpp"

#include <span>

namespace lamavm::interpreter {

template <verify_kind kind>
std::span<box> closure_payload(box closure);

extern template std::span<box> closure_payload<hard>(box closure);
extern template std::span<box> closure_payload<soft>(box closure);

} // namespace lamavm::interpreter
