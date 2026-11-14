#include "std.hpp"

#include "box.hpp"

#include <bit>

namespace lamavm::interpreter {

namespace cc {

extern "C" {

void* Ls__Infix_43(void* x, void* y);   // NOLINT
void* Ls__Infix_45(void* x, void* y);   // NOLINT
void* Ls__Infix_42(void* x, void* y);   // NOLINT
void* Ls__Infix_47(void* x, void* y);   // NOLINT
void* Ls__Infix_37(void* x, void* y);   // NOLINT
void* Ls__Infix_60(void* x, void* y);   // NOLINT
void* Ls__Infix_6061(void* x, void* y); // NOLINT
void* Ls__Infix_62(void* x, void* y);   // NOLINT
void* Ls__Infix_6261(void* x, void* y); // NOLINT
void* Ls__Infix_6161(void* x, void* y); // NOLINT
void* Ls__Infix_3361(void* x, void* y); // NOLINT
void* Ls__Infix_3838(void* x, void* y); // NOLINT
void* Ls__Infix_3333(void* x, void* y); // NOLINT
void* Bstring(void* args);              // NOLINT
}

} // namespace cc

namespace {

template <auto f>
box binop(box x, box y) {
  return box(f(std::bit_cast<void*>(x), std::bit_cast<void*>(y)));
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
  return box(cc::Bstring(unbox<void*>(x)));
}

} // namespace lamavm::interpreter
