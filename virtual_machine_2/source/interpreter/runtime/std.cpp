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

void* Bstring(aint* args);           // NOLINT
aint LtagHash(const char* s);        // NOLINT
void* Bsexp(aint* args, aint bn);    // NOLINT
aint Btag(void* d, aint t, aint n);  // NOLINT
void* Belem(void* p, aint i);        // NOLINT
void* Barray(aint* args, aint bn);   // NOLINT
aint Barray_patt(void* d, aint n);   // NOLINT
void* Bclosure(aint* args, aint bn); // NOLINT
aint Bstring_patt(void* x, void* y); // NOLINT
aint Bstring_tag_patt(void* x);      // NOLINT
aint Barray_tag_patt(void* x);       // NOLINT
aint Bsexp_tag_patt(void* x);        // NOLINT
aint Bboxed_patt(void* x);           // NOLINT
aint Bunboxed_patt(void* x);         // NOLINT
aint Bclosure_tag_patt(void* x);     // NOLINT

void Bmatch_failure(void* v, char* fname, aint line, aint col); // NOLINT
}

} // namespace cc

namespace {

template <auto f>
box unop(box x) {
  auto* const x1 = std::bit_cast<void*>(x);
  const auto r = f(x1);
  return std::bit_cast<box>(r);
}

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
  aint* x1 = std::bit_cast<aint*>(x);
  const void* r = cc::Bstring(x1);
  return std::bit_cast<box>(r);
}

box LtagHash(box x) {
  const char* x1 = std::bit_cast<const char*>(x);
  const aint r = cc::LtagHash(x1);
  return std::bit_cast<box>(r);
}

box Bsexp(box args, box bn) {
  aint* args1 = std::bit_cast<aint*>(args);
  const aint bn1 = std::bit_cast<aint>(bn);
  const void* r = cc::Bsexp(args1, bn1);
  return std::bit_cast<box>(r);
}

box Btag(box d, box t, box n) {
  void* d1 = std::bit_cast<void*>(d);
  const aint t1 = std::bit_cast<aint>(t);
  const aint n1 = std::bit_cast<aint>(n);
  const aint r = cc::Btag(d1, t1, n1);
  return std::bit_cast<box>(r);
}

box Belem(box p, box i) {
  void* p1 = std::bit_cast<void*>(p);
  const aint i1 = std::bit_cast<aint>(i);
  const void* r = cc::Belem(p1, i1);
  return std::bit_cast<box>(r);
}

box Barray(box args, box bn) {
  aint* args1 = std::bit_cast<aint*>(args);
  const aint bn1 = std::bit_cast<aint>(bn);
  const void* r = cc::Barray(args1, bn1);
  return std::bit_cast<box>(r);
}

box Barray_patt(box d, box n) {
  void* d1 = std::bit_cast<void*>(d);
  const aint n1 = std::bit_cast<aint>(n);
  const aint r = cc::Barray_patt(d1, n1);
  return std::bit_cast<box>(r);
}

box Bclosure(box args, box bn) {
  aint* args1 = std::bit_cast<aint*>(args);
  const aint bn1 = std::bit_cast<aint>(bn);
  const void* r = cc::Bclosure(args1, bn1);
  return std::bit_cast<box>(r);
}

box Bstring_patt(box x, box y) {
  return binop<cc::Bstring_patt>(x, y);
}

box Bstring_tag_patt(box x) {
  return unop<cc::Bstring_tag_patt>(x);
}

box Barray_tag_patt(box x) {
  return unop<cc::Barray_tag_patt>(x);
}

box Bsexp_tag_patt(box x) {
  return unop<cc::Bsexp_tag_patt>(x);
}

box Bboxed_patt(box x) {
  return unop<cc::Bboxed_patt>(x);
}

box Bunboxed_patt(box x) {
  return unop<cc::Bunboxed_patt>(x);
}

box Bclosure_tag_patt(box x) {
  return unop<cc::Bclosure_tag_patt>(x);
}

void Bmatch_failure(box v, box fname, box line, box col) {
  void* v1 = std::bit_cast<void*>(v);
  char* fname1 = std::bit_cast<char*>(fname);
  const aint line1 = std::bit_cast<aint>(line);
  const aint col1 = std::bit_cast<aint>(col);
  cc::Bmatch_failure(v1, fname1, line1, col1);
}

} // namespace lamavm::interpreter
