#include "std.hpp"

#include "box.hpp"

#include <contrib/runtime/runtime_common.h>

#include <bit>

namespace lamavm::interpreter {

namespace cc {

extern "C" {
aint Ls__Infix_43(void* x, void* y);   // NOLINT
aint Ls__Infix_45(void* x, void* y);   // NOLINT
aint Ls__Infix_42(void* x, void* y);   // NOLINT
aint Ls__Infix_47(void* x, void* y);   // NOLINT
aint Ls__Infix_37(void* x, void* y);   // NOLINT
aint Ls__Infix_60(void* x, void* y);   // NOLINT
aint Ls__Infix_6061(void* x, void* y); // NOLINT
aint Ls__Infix_62(void* x, void* y);   // NOLINT
aint Ls__Infix_6261(void* x, void* y); // NOLINT
aint Ls__Infix_6161(void* x, void* y); // NOLINT
aint Ls__Infix_3361(void* x, void* y); // NOLINT
aint Ls__Infix_3838(void* x, void* y); // NOLINT
aint Ls__Infix_3333(void* x, void* y); // NOLINT

void* Bstring(aint* args); // NOLINT
}

} // namespace cc

namespace {

template <auto f>
box binop(box x, box y) {
  auto* const x1 = std::bit_cast<void*>(x);
  auto* const y1 = std::bit_cast<void*>(y);
  const auto r1 = f(x1, y1);
  return std::bit_cast<box>(r1);
}

} // namespace

box Ls__Infix_43(box x, box y) {
  return binop<cc::Ls__Infix_43>(x, y);
}

box Ls__Infix_45(box x, box y) {
  return binop<cc::Ls__Infix_45>(x, y);
}

box Ls__Infix_42(box x, box y) {
  return binop<cc::Ls__Infix_42>(x, y);
}

box Ls__Infix_47(box x, box y) {
  return binop<cc::Ls__Infix_47>(x, y);
}

box Ls__Infix_37(box x, box y) {
  return binop<cc::Ls__Infix_37>(x, y);
}

box Ls__Infix_60(box x, box y) {
  return binop<cc::Ls__Infix_60>(x, y);
}

box Ls__Infix_6061(box x, box y) {
  return binop<cc::Ls__Infix_6061>(x, y);
}

box Ls__Infix_62(box x, box y) {
  return binop<cc::Ls__Infix_62>(x, y);
}

box Ls__Infix_6261(box x, box y) {
  return binop<cc::Ls__Infix_6261>(x, y);
}

box Ls__Infix_6161(box x, box y) {
  return binop<cc::Ls__Infix_6161>(x, y);
}

box Ls__Infix_3361(box x, box y) {
  return binop<cc::Ls__Infix_3361>(x, y);
}

box Ls__Infix_3838(box x, box y) {
  return binop<cc::Ls__Infix_3838>(x, y);
}

box Ls__Infix_3333(box x, box y) {
  return binop<cc::Ls__Infix_3333>(x, y);
}

box Bstring(box x) {
  aint* x1 = unbox<aint*>(x);
  const void* r = cc::Bstring(x1);
  return box(r);
}

} // namespace lamavm::interpreter
