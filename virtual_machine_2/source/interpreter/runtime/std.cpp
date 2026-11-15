#include "std.hpp"

#include "box.hpp"

#include <core/log.hpp>

#include <contrib/runtime/runtime_common.h>

#include <bit>
#include <cstdio>
#include <regex.h>
#include <type_traits>

namespace lamavm::interpreter {

namespace cc {

extern "C" {

[[noreturn]] void failure(char* s, ...);
void Lassert(void* f, char* s, ...);                            // NOLINT
aint LkindOf(void* p);                                          // NOLINT
aint LcompareTags(void* p, void* q);                            // NOLINT
void* Ls__Infix_58(void** args);                                // NOLINT
aint Ls__Infix_3333(void* p, void* q);                          // NOLINT
aint Ls__Infix_3838(void* p, void* q);                          // NOLINT
aint Ls__Infix_6161(void* p, void* q);                          // NOLINT
aint Ls__Infix_3361(void* p, void* q);                          // NOLINT
aint Ls__Infix_6061(void* p, void* q);                          // NOLINT
aint Ls__Infix_60(void* p, void* q);                            // NOLINT
aint Ls__Infix_6261(void* p, void* q);                          // NOLINT
aint Ls__Infix_62(void* p, void* q);                            // NOLINT
aint Ls__Infix_43(void* p, void* q);                            // NOLINT
aint Ls__Infix_45(void* p, void* q);                            // NOLINT
aint Ls__Infix_42(void* p, void* q);                            // NOLINT
aint Ls__Infix_47(void* p, void* q);                            // NOLINT
aint Ls__Infix_37(void* p, void* q);                            // NOLINT
aint Llength(void* p);                                          // NOLINT
aint LtagHash(char* s);                                         // NOLINT
aint Luppercase(void* v);                                       // NOLINT
aint Llowercase(void* v);                                       // NOLINT
aint LmatchSubString(char* subj, char* patt, aint pos);         // NOLINT
void* Lsubstring(aint* args);                                   // NOLINT
regex_t* Lregexp(char* regexp);                                 // NOLINT
aint LregexpMatch(regex_t* b, char* s, aint pos);               // NOLINT
void* Lclone(aint* args);                                       // NOLINT
void* LstringInt(char* b);                                      // NOLINT
aint Lhash(void* p);                                            // NOLINT
aint LflatCompare(void* p, void* q);                            // NOLINT
aint Lcompare(void* p, void* q);                                // NOLINT
void* Belem(void* p, aint i);                                   // NOLINT
void* LmakeArray(aint length);                                  // NOLINT
void* LmakeString(aint length);                                 // NOLINT
void* Bstring(aint* args);                                      // NOLINT
void* Lstringcat(aint* args);                                   // NOLINT
void* Lstring(aint* args);                                      // NOLINT
void* Bclosure(aint* args, aint bn);                            // NOLINT
void* Barray(aint* args, aint bn);                              // NOLINT
void* Bsexp(aint* args, aint bn);                               // NOLINT
aint Btag(void* d, aint t, aint n);                             // NOLINT
aint Barray_patt(void* d, aint n);                              // NOLINT
aint Bstring_patt(void* x, void* y);                            // NOLINT
aint Bclosure_tag_patt(void* x);                                // NOLINT
aint Bboxed_patt(void* x);                                      // NOLINT
aint Bunboxed_patt(void* x);                                    // NOLINT
aint Barray_tag_patt(void* x);                                  // NOLINT
aint Bstring_tag_patt(void* x);                                 // NOLINT
aint Bsexp_tag_patt(void* x);                                   // NOLINT
void* Bsta(void* x, aint i, void* v);                           // NOLINT
void Bmatch_failure(void* v, char* fname, aint line, aint col); // NOLINT
void* Li__Infix_4343(aint* args);                               // NOLINT
void* LgetEnv(char* var);                                       // NOLINT
aint Lsystem(char* cmd);                                        // NOLINT
void* Bsprintf(char* fmt, ...);                                 // NOLINT
void Bprintf(char* s, ...);                                     // NOLINT
void Bfprintf(FILE* f, char* s, ...);                           // NOLINT
FILE* Lfopen(char* f, char* m);                                 // NOLINT
void Lfclose(FILE* f);                                          // NOLINT
void* LreadLine();                                              // NOLINT
void* Lfread(char* fname);                                      // NOLINT
void Lfwrite(char* fname, char* contents);                      // NOLINT
void* Lfexists(char* fname);                                    // NOLINT
void* Lfst(void* v);                                            // NOLINT
void* Lsnd(void* v);                                            // NOLINT
void* Lhd(void* v);                                             // NOLINT
void* Ltl(void* v);                                             // NOLINT
aint Lread();                                                   // NOLINT
aint Lwrite(aint n);                                            // NOLINT
aint Lrandom(aint n);                                           // NOLINT
aint Ltime();                                                   // NOLINT
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

// NOLINTNEXTLINE(modernize-avoid-variadic-functions)
[[noreturn]] void failure([[maybe_unused]] box s, ...) {
  fatalf("unsupported variadic runtime function: failure");
}

// NOLINTNEXTLINE(modernize-avoid-variadic-functions)
void Lassert([[maybe_unused]] box f, [[maybe_unused]] box s, ...) {
  fatalf("unsupported variadic runtime function: Lassert");
}

box LkindOf(box p) {
  return bridge<aint, cc::LkindOf, void*>(p);
}

box LcompareTags(box p, box q) {
  return bridge<aint, cc::LcompareTags, void*, void*>(p, q);
}

box Ls__Infix_58(box args) {
  return bridge<void*, cc::Ls__Infix_58, void**>(args);
}

box Ls__Infix_3333(box p, box q) {
  return bridge<aint, cc::Ls__Infix_3333, void*, void*>(p, q);
}

box Ls__Infix_3838(box p, box q) {
  return bridge<aint, cc::Ls__Infix_3838, void*, void*>(p, q);
}

box Ls__Infix_6161(box p, box q) {
  return bridge<aint, cc::Ls__Infix_6161, void*, void*>(p, q);
}

box Ls__Infix_3361(box p, box q) {
  return bridge<aint, cc::Ls__Infix_3361, void*, void*>(p, q);
}

box Ls__Infix_6061(box p, box q) {
  return bridge<aint, cc::Ls__Infix_6061, void*, void*>(p, q);
}

box Ls__Infix_60(box p, box q) {
  return bridge<aint, cc::Ls__Infix_60, void*, void*>(p, q);
}

box Ls__Infix_6261(box p, box q) {
  return bridge<aint, cc::Ls__Infix_6261, void*, void*>(p, q);
}

box Ls__Infix_62(box p, box q) {
  return bridge<aint, cc::Ls__Infix_62, void*, void*>(p, q);
}

box Ls__Infix_43(box p, box q) {
  return bridge<aint, cc::Ls__Infix_43, void*, void*>(p, q);
}

box Ls__Infix_45(box p, box q) {
  return bridge<aint, cc::Ls__Infix_45, void*, void*>(p, q);
}

box Ls__Infix_42(box p, box q) {
  return bridge<aint, cc::Ls__Infix_42, void*, void*>(p, q);
}

box Ls__Infix_47(box p, box q) {
  return bridge<aint, cc::Ls__Infix_47, void*, void*>(p, q);
}

box Ls__Infix_37(box p, box q) {
  return bridge<aint, cc::Ls__Infix_37, void*, void*>(p, q);
}

box Llength(box p) {
  return bridge<aint, cc::Llength, void*>(p);
}

box LtagHash(box s) {
  return bridge<aint, cc::LtagHash, char*>(s);
}

box Luppercase(box v) {
  return bridge<aint, cc::Luppercase, void*>(v);
}

box Llowercase(box v) {
  return bridge<aint, cc::Llowercase, void*>(v);
}

box LmatchSubString(box subj, box patt, box pos) {
  return bridge<aint, cc::LmatchSubString, char*, char*, aint>(subj, patt, pos);
}

box Lsubstring(box args) {
  return bridge<void*, cc::Lsubstring, aint*>(args);
}

box Lregexp(box regexp) {
  return bridge<regex_t*, cc::Lregexp, char*>(regexp);
}

box LregexpMatch(box b, box s, box pos) {
  return bridge<aint, cc::LregexpMatch, regex_t*, char*, aint>(b, s, pos);
}

box Lclone(box args) {
  return bridge<void*, cc::Lclone, aint*>(args);
}

box LstringInt(box b) {
  return bridge<void*, cc::LstringInt, char*>(b);
}

box Lhash(box p) {
  return bridge<aint, cc::Lhash, void*>(p);
}

box LflatCompare(box p, box q) {
  return bridge<aint, cc::LflatCompare, void*, void*>(p, q);
}

box Lcompare(box p, box q) {
  return bridge<aint, cc::Lcompare, void*, void*>(p, q);
}

box Belem(box p, box i) {
  return bridge<void*, cc::Belem, void*, aint>(p, i);
}

box LmakeArray(box length) {
  return bridge<void*, cc::LmakeArray, aint>(length);
}

box LmakeString(box length) {
  return bridge<void*, cc::LmakeString, aint>(length);
}

box Bstring(box args) {
  return bridge<void*, cc::Bstring, aint*>(args);
}

box Lstringcat(box args) {
  return bridge<void*, cc::Lstringcat, aint*>(args);
}

box Lstring(box args) {
  return bridge<void*, cc::Lstring, aint*>(args);
}

box Bclosure(box args, box bn) {
  return bridge<void*, cc::Bclosure, aint*, aint>(args, bn);
}

box Barray(box args, box bn) {
  return bridge<void*, cc::Barray, aint*, aint>(args, bn);
}

box Bsexp(box args, box bn) {
  return bridge<void*, cc::Bsexp, aint*, aint>(args, bn);
}

box Btag(box d, box t, box n) {
  return bridge<aint, cc::Btag, void*, aint, aint>(d, t, n);
}

box Barray_patt(box d, box n) {
  return bridge<aint, cc::Barray_patt, void*, aint>(d, n);
}

box Bstring_patt(box x, box y) {
  return bridge<aint, cc::Bstring_patt, void*, void*>(x, y);
}

box Bclosure_tag_patt(box x) {
  return bridge<aint, cc::Bclosure_tag_patt, void*>(x);
}

box Bboxed_patt(box x) {
  return bridge<aint, cc::Bboxed_patt, void*>(x);
}

box Bunboxed_patt(box x) {
  return bridge<aint, cc::Bunboxed_patt, void*>(x);
}

box Barray_tag_patt(box x) {
  return bridge<aint, cc::Barray_tag_patt, void*>(x);
}

box Bstring_tag_patt(box x) {
  return bridge<aint, cc::Bstring_tag_patt, void*>(x);
}

box Bsexp_tag_patt(box x) {
  return bridge<aint, cc::Bsexp_tag_patt, void*>(x);
}

box Bsta(box x, box i, box v) {
  return bridge<void*, cc::Bsta, void*, aint, void*>(x, i, v);
}

void Bmatch_failure(box v, box fname, box line, box col) {
  bridge<void, cc::Bmatch_failure, void*, char*, aint, aint>(v, fname, line, col);
}

box Li__Infix_4343(box args) {
  return bridge<void*, cc::Li__Infix_4343, aint*>(args);
}

box LgetEnv(box var) {
  return bridge<void*, cc::LgetEnv, char*>(var);
}

box Lsystem(box cmd) {
  return bridge<aint, cc::Lsystem, char*>(cmd);
}

box Bsprintf([[maybe_unused]] box fmt, ...) { // NOLINT
  fatalf("unsupported variadic runtime function: Bsprintf");
}

void Bprintf([[maybe_unused]] box s, ...) { // NOLINT
  fatalf("unsupported variadic runtime function: Bprintf");
}

void Bfprintf([[maybe_unused]] box f, [[maybe_unused]] box s, ...) { // NOLINT
  fatalf("unsupported variadic runtime function: Bfprintf");
}

box Lfopen(box f, box m) {
  return bridge<FILE*, cc::Lfopen, char*, char*>(f, m);
}

void Lfclose(box f) {
  bridge<void, cc::Lfclose, FILE*>(f);
}

box LreadLine() {
  return bridge<void*, cc::LreadLine>();
}

box Lfread(box fname) {
  return bridge<void*, cc::Lfread, char*>(fname);
}

void Lfwrite(box fname, box contents) {
  bridge<void, cc::Lfwrite, char*, char*>(fname, contents);
}

box Lfexists(box fname) {
  return bridge<void*, cc::Lfexists, char*>(fname);
}

box Lfst(box v) {
  return bridge<void*, cc::Lfst, void*>(v);
}

box Lsnd(box v) {
  return bridge<void*, cc::Lsnd, void*>(v);
}

box Lhd(box v) {
  return bridge<void*, cc::Lhd, void*>(v);
}

box Ltl(box v) {
  return bridge<void*, cc::Ltl, void*>(v);
}

box Lread() {
  return bridge<aint, cc::Lread>();
}

box Lwrite(box n) {
  return bridge<aint, cc::Lwrite, aint>(n);
}

box Lrandom(box n) {
  return bridge<aint, cc::Lrandom, aint>(n);
}

box Ltime() {
  return bridge<aint, cc::Ltime>();
}

} // namespace lamavm::interpreter
