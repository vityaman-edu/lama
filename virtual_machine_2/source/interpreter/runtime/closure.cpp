#include "closure.hpp"

#include "box.hpp"
#include "core/assert.hpp"
#include "interpreter/std/std.g.hpp"
#include "interpreter/std/std.hpp"

#include <cstddef>
#include <cstdint>
#include <span>

namespace lamavm::interpreter {

template <verify_kind kind>
std::span<box> closure_payload(box closure) {
  auto* payload = unbox<box*>(closure);
  LAMAVM_VERIFY_C(kind, payload != nullptr, "expected a closure on the stack, got null");
  LAMAVM_VERIFY_C(
    kind,
    unbox<std::int64_t>(boxd::Bclosure_tag_patt(closure)) == 1,
    "expected a closure on the stack");

  const auto size = unbox<std::int64_t>(boxd::std_length(closure));
  LAMAVM_VERIFY_B(kind, 2, <=, size);

  return {payload, static_cast<std::size_t>(size)};
}

template std::span<box> closure_payload<hard>(box closure);
template std::span<box> closure_payload<soft>(box closure);

} // namespace lamavm::interpreter
