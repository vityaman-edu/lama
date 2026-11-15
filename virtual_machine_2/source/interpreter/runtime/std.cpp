#include "std.hpp"

#include "box.hpp"

#include <contrib/runtime/runtime_common.h>

#include <bit>
#include <type_traits>

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

void* Bstring(aint* args);            // NOLINT
aint LtagHash(const char* s);         // NOLINT
void* Bsexp(aint* args, aint bn);     // NOLINT
aint Btag(void* d, aint t, aint n);   // NOLINT
void* Belem(void* p, aint i);         // NOLINT
void* Bsta(void* x, aint i, void* v); // NOLINT
void* Barray(aint* args, aint bn);    // NOLINT
aint Barray_patt(void* d, aint n);    // NOLINT
void* Bclosure(aint* args, aint bn);  // NOLINT
aint Bstring_patt(void* x, void* y);  // NOLINT
aint Bstring_tag_patt(void* x);       // NOLINT
aint Barray_tag_patt(void* x);        // NOLINT
aint Bsexp_tag_patt(void* x);         // NOLINT
aint Bboxed_patt(void* x);            // NOLINT
aint Bunboxed_patt(void* x);          // NOLINT
aint Bclosure_tag_patt(void* x);      // NOLINT

void Bmatch_failure(void* v, char* fname, aint line, aint col); // NOLINT
}

} // namespace cc

namespace {

template <typename R, auto f, typename... T>
std::conditional_t<std::is_void_v<R>, void, box> bridge(std::conditional_t<true, box, T>... args) {
  if constexpr (std::is_void_v<R>) {
    f(std::bit_cast<T>(args)...);
  } else {
    return std::bit_cast<box>(f(std::bit_cast<T>(args)...));
  }
}

} // namespace

box Ls__Infix_43(box x, box y) {
  return bridge<aint, cc::Ls__Infix_43, void*, void*>(x, y);
}

box Ls__Infix_45(box x, box y) {
  return bridge<aint, cc::Ls__Infix_45, void*, void*>(x, y);
}

box Ls__Infix_42(box x, box y) {
  return bridge<aint, cc::Ls__Infix_42, void*, void*>(x, y);
}

box Ls__Infix_47(box x, box y) {
  return bridge<aint, cc::Ls__Infix_47, void*, void*>(x, y);
}

box Ls__Infix_37(box x, box y) {
  return bridge<aint, cc::Ls__Infix_37, void*, void*>(x, y);
}

box Ls__Infix_60(box x, box y) {
  return bridge<aint, cc::Ls__Infix_60, void*, void*>(x, y);
}

box Ls__Infix_6061(box x, box y) {
  return bridge<aint, cc::Ls__Infix_6061, void*, void*>(x, y);
}

box Ls__Infix_62(box x, box y) {
  return bridge<aint, cc::Ls__Infix_62, void*, void*>(x, y);
}

box Ls__Infix_6261(box x, box y) {
  return bridge<aint, cc::Ls__Infix_6261, void*, void*>(x, y);
}

box Ls__Infix_6161(box x, box y) {
  return bridge<aint, cc::Ls__Infix_6161, void*, void*>(x, y);
}

box Ls__Infix_3361(box x, box y) {
  return bridge<aint, cc::Ls__Infix_3361, void*, void*>(x, y);
}

box Ls__Infix_3838(box x, box y) {
  return bridge<aint, cc::Ls__Infix_3838, void*, void*>(x, y);
}

box Ls__Infix_3333(box x, box y) {
  return bridge<aint, cc::Ls__Infix_3333, void*, void*>(x, y);
}

box Bstring(box x) {
  return bridge<void*, cc::Bstring, aint*>(x);
}

box LtagHash(box x) {
  return bridge<aint, cc::LtagHash, const char*>(x);
}

box Bsexp(box args, box bn) {
  return bridge<void*, cc::Bsexp, aint*, aint>(args, bn);
}

box Btag(box d, box t, box n) {
  return bridge<aint, cc::Btag, void*, aint, aint>(d, t, n);
}

box Belem(box p, box i) {
  return bridge<void*, cc::Belem, void*, aint>(p, i);
}

box Bsta(box x, box i, box v) {
  return bridge<void*, cc::Bsta, void*, aint, void*>(x, i, v);
}

box Barray(box args, box bn) {
  return bridge<void*, cc::Barray, aint*, aint>(args, bn);
}

box Barray_patt(box d, box n) {
  return bridge<aint, cc::Barray_patt, void*, aint>(d, n);
}

box Bclosure(box args, box bn) {
  return bridge<void*, cc::Bclosure, aint*, aint>(args, bn);
}

box Bstring_patt(box x, box y) {
  return bridge<aint, cc::Bstring_patt, void*, void*>(x, y);
}

box Bstring_tag_patt(box x) {
  return bridge<aint, cc::Bstring_tag_patt, void*>(x);
}

box Barray_tag_patt(box x) {
  return bridge<aint, cc::Barray_tag_patt, void*>(x);
}

box Bsexp_tag_patt(box x) {
  return bridge<aint, cc::Bsexp_tag_patt, void*>(x);
}

box Bboxed_patt(box x) {
  return bridge<aint, cc::Bboxed_patt, void*>(x);
}

box Bunboxed_patt(box x) {
  return bridge<aint, cc::Bunboxed_patt, void*>(x);
}

box Bclosure_tag_patt(box x) {
  return bridge<aint, cc::Bclosure_tag_patt, void*>(x);
}

void Bmatch_failure(box v, box fname, box line, box col) {
  bridge<void, cc::Bmatch_failure, void*, char*, aint, aint>(v, fname, line, col);
}

} // namespace lamavm::interpreter
