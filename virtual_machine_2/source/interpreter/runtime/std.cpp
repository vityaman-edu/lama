#include "std.hpp"

#include "box.hpp"

#include <core/assert.hpp>
#include <core/functional.hpp>
#include <core/log.hpp>

#include <contrib/runtime/runtime_common.h>

#include <bit>
#include <cstdio>
#include <regex.h>
#include <span>
#include <type_traits>
#include <utility>

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

template <auto f, typename R, typename... T>
std::conditional_t<std::is_void_v<R>, void, box>
bridge_impl([[maybe_unused]] R (*native)(T...), std::conditional_t<true, box, T>... args) {
  if constexpr (std::is_void_v<R>) {
    f(std::bit_cast<T>(args)...);
  } else {
    return std::bit_cast<box>(f(std::bit_cast<T>(args)...));
  }
}

template <auto f, typename... T>
auto bridge(T... args) {
  return bridge_impl<f>(f, args...);
}

template <typename F, std::size_t... i>
function_traits<F>::return_type invoke_std_impl(
  std::span<const box> rargs,
  F f,
  [[maybe_unused]] std::index_sequence<i...> indices) {
  if constexpr (std::is_void_v<typename function_traits<F>::return_type>) {
    f(rargs[rargs.size() - 1 - i]...);
  } else {
    return f(rargs[rargs.size() - 1 - i]...);
  }
}

template <typename F>
function_traits<F>::return_type invoke_std_impl(std::span<const box> rargs, F f) {
  using traits = function_traits<F>;
  if constexpr (traits::is_variadic) {
    LAMAVM_ASSERT(rargs.size() >= traits::arity);
  } else {
    LAMAVM_ASSERT(rargs.size() == traits::arity);
  }

  return invoke_std_impl(rargs, f, std::make_index_sequence<traits::arity>{});
}

template <typename R>
R invoke_std(std::span<const box> rargs, R (*f)()) {
  return invoke_std_impl(rargs, f);
}

template <typename R, typename... T>
R invoke_std(std::span<const box> rargs, R (*f)(box, T...)) {
  return invoke_std_impl(rargs, f);
}

template <typename R, typename... T>
R invoke_std(std::span<const box> rargs, R (*f)(box, T..., ...)) {
  return invoke_std_impl(rargs, f);
}

} // namespace

namespace boxd {

// NOLINTNEXTLINE(modernize-avoid-variadic-functions)
[[noreturn]] void failure([[maybe_unused]] box s, ...) {
  fatalf("unsupported variadic runtime function: failure");
}

// NOLINTNEXTLINE(modernize-avoid-variadic-functions)
void Lassert([[maybe_unused]] box f, [[maybe_unused]] box s, ...) {
  fatalf("unsupported variadic runtime function: Lassert");
}

box LkindOf(box p) {
  return bridge<cc::LkindOf>(p);
}

box LcompareTags(box p, box q) {
  return bridge<cc::LcompareTags>(p, q);
}

box Ls__Infix_58(box args) {
  return bridge<cc::Ls__Infix_58>(args);
}

box Ls__Infix_3333(box p, box q) {
  return bridge<cc::Ls__Infix_3333>(p, q);
}

box Ls__Infix_3838(box p, box q) {
  return bridge<cc::Ls__Infix_3838>(p, q);
}

box Ls__Infix_6161(box p, box q) {
  return bridge<cc::Ls__Infix_6161>(p, q);
}

box Ls__Infix_3361(box p, box q) {
  return bridge<cc::Ls__Infix_3361>(p, q);
}

box Ls__Infix_6061(box p, box q) {
  return bridge<cc::Ls__Infix_6061>(p, q);
}

box Ls__Infix_60(box p, box q) {
  return bridge<cc::Ls__Infix_60>(p, q);
}

box Ls__Infix_6261(box p, box q) {
  return bridge<cc::Ls__Infix_6261>(p, q);
}

box Ls__Infix_62(box p, box q) {
  return bridge<cc::Ls__Infix_62>(p, q);
}

box Ls__Infix_43(box p, box q) {
  return bridge<cc::Ls__Infix_43>(p, q);
}

box Ls__Infix_45(box p, box q) {
  return bridge<cc::Ls__Infix_45>(p, q);
}

box Ls__Infix_42(box p, box q) {
  return bridge<cc::Ls__Infix_42>(p, q);
}

box Ls__Infix_47(box p, box q) {
  return bridge<cc::Ls__Infix_47>(p, q);
}

box Ls__Infix_37(box p, box q) {
  return bridge<cc::Ls__Infix_37>(p, q);
}

box Llength(box p) {
  return bridge<cc::Llength>(p);
}

box LtagHash(box s) {
  return bridge<cc::LtagHash>(s);
}

box Luppercase(box v) {
  return bridge<cc::Luppercase>(v);
}

box Llowercase(box v) {
  return bridge<cc::Llowercase>(v);
}

box LmatchSubString(box subj, box patt, box pos) {
  return bridge<cc::LmatchSubString>(subj, patt, pos);
}

box Lsubstring(box args) {
  return bridge<cc::Lsubstring>(args);
}

box Lregexp(box regexp) {
  return bridge<cc::Lregexp>(regexp);
}

box LregexpMatch(box b, box s, box pos) {
  return bridge<cc::LregexpMatch>(b, s, pos);
}

box Lclone(box args) {
  return bridge<cc::Lclone>(args);
}

box LstringInt(box b) {
  return bridge<cc::LstringInt>(b);
}

box Lhash(box p) {
  return bridge<cc::Lhash>(p);
}

box LflatCompare(box p, box q) {
  return bridge<cc::LflatCompare>(p, q);
}

box Lcompare(box p, box q) {
  return bridge<cc::Lcompare>(p, q);
}

box Belem(box p, box i) {
  return bridge<cc::Belem>(p, i);
}

box LmakeArray(box length) {
  return bridge<cc::LmakeArray>(length);
}

box LmakeString(box length) {
  return bridge<cc::LmakeString>(length);
}

box Bstring(box args) {
  return bridge<cc::Bstring>(args);
}

box Lstringcat(box args) {
  return bridge<cc::Lstringcat>(args);
}

box Lstring(box args) {
  return bridge<cc::Lstring>(args);
}

box Bclosure(box args, box bn) {
  return bridge<cc::Bclosure>(args, bn);
}

box Barray(box args, box bn) {
  return bridge<cc::Barray>(args, bn);
}

box Bsexp(box args, box bn) {
  return bridge<cc::Bsexp>(args, bn);
}

box Btag(box d, box t, box n) {
  return bridge<cc::Btag>(d, t, n);
}

box Barray_patt(box d, box n) {
  return bridge<cc::Barray_patt>(d, n);
}

box Bstring_patt(box x, box y) {
  return bridge<cc::Bstring_patt>(x, y);
}

box Bclosure_tag_patt(box x) {
  return bridge<cc::Bclosure_tag_patt>(x);
}

box Bboxed_patt(box x) {
  return bridge<cc::Bboxed_patt>(x);
}

box Bunboxed_patt(box x) {
  return bridge<cc::Bunboxed_patt>(x);
}

box Barray_tag_patt(box x) {
  return bridge<cc::Barray_tag_patt>(x);
}

box Bstring_tag_patt(box x) {
  return bridge<cc::Bstring_tag_patt>(x);
}

box Bsexp_tag_patt(box x) {
  return bridge<cc::Bsexp_tag_patt>(x);
}

box Bsta(box x, box i, box v) {
  return bridge<cc::Bsta>(x, i, v);
}

void Bmatch_failure(box v, box fname, box line, box col) {
  bridge<cc::Bmatch_failure>(v, fname, line, col);
}

box Li__Infix_4343(box args) {
  return bridge<cc::Li__Infix_4343>(args);
}

box LgetEnv(box var) {
  return bridge<cc::LgetEnv>(var);
}

box Lsystem(box cmd) {
  return bridge<cc::Lsystem>(cmd);
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
  return bridge<cc::Lfopen>(f, m);
}

void Lfclose(box f) {
  bridge<cc::Lfclose>(f);
}

box LreadLine() {
  return bridge<cc::LreadLine>();
}

box Lfread(box fname) {
  return bridge<cc::Lfread>(fname);
}

void Lfwrite(box fname, box contents) {
  bridge<cc::Lfwrite>(fname, contents);
}

box Lfexists(box fname) {
  return bridge<cc::Lfexists>(fname);
}

box Lfst(box v) {
  return bridge<cc::Lfst>(v);
}

box Lsnd(box v) {
  return bridge<cc::Lsnd>(v);
}

box Lhd(box v) {
  return bridge<cc::Lhd>(v);
}

box Ltl(box v) {
  return bridge<cc::Ltl>(v);
}

box Lread() {
  return bridge<cc::Lread>();
}

box Lwrite(box n) {
  return bridge<cc::Lwrite>(n);
}

box Lrandom(box n) {
  return bridge<cc::Lrandom>(n);
}

box Ltime() {
  return bridge<cc::Ltime>();
}

} // namespace boxd

namespace span {

void failure(std::span<const box> rargs) {
  invoke_std(rargs, boxd::failure);
}

void Lassert(std::span<const box> rargs) {
  invoke_std(rargs, boxd::Lassert);
}

box LkindOf(std::span<const box> rargs) {
  return invoke_std(rargs, boxd::LkindOf);
}

box LcompareTags(std::span<const box> rargs) {
  return invoke_std(rargs, boxd::LcompareTags);
}

box Ls__Infix_58(std::span<const box> rargs) {
  return invoke_std(rargs, boxd::Ls__Infix_58);
}

box Ls__Infix_3333(std::span<const box> rargs) {
  return invoke_std(rargs, boxd::Ls__Infix_3333);
}

box Ls__Infix_3838(std::span<const box> rargs) {
  return invoke_std(rargs, boxd::Ls__Infix_3838);
}

box Ls__Infix_6161(std::span<const box> rargs) {
  return invoke_std(rargs, boxd::Ls__Infix_6161);
}

box Ls__Infix_3361(std::span<const box> rargs) {
  return invoke_std(rargs, boxd::Ls__Infix_3361);
}

box Ls__Infix_6061(std::span<const box> rargs) {
  return invoke_std(rargs, boxd::Ls__Infix_6061);
}

box Ls__Infix_60(std::span<const box> rargs) {
  return invoke_std(rargs, boxd::Ls__Infix_60);
}

box Ls__Infix_6261(std::span<const box> rargs) {
  return invoke_std(rargs, boxd::Ls__Infix_6261);
}

box Ls__Infix_62(std::span<const box> rargs) {
  return invoke_std(rargs, boxd::Ls__Infix_62);
}

box Ls__Infix_43(std::span<const box> rargs) {
  return invoke_std(rargs, boxd::Ls__Infix_43);
}

box Ls__Infix_45(std::span<const box> rargs) {
  return invoke_std(rargs, boxd::Ls__Infix_45);
}

box Ls__Infix_42(std::span<const box> rargs) {
  return invoke_std(rargs, boxd::Ls__Infix_42);
}

box Ls__Infix_47(std::span<const box> rargs) {
  return invoke_std(rargs, boxd::Ls__Infix_47);
}

box Ls__Infix_37(std::span<const box> rargs) {
  return invoke_std(rargs, boxd::Ls__Infix_37);
}

box Llength(std::span<const box> rargs) {
  return invoke_std(rargs, boxd::Llength);
}

box LtagHash(std::span<const box> rargs) {
  return invoke_std(rargs, boxd::LtagHash);
}

box Luppercase(std::span<const box> rargs) {
  return invoke_std(rargs, boxd::Luppercase);
}

box Llowercase(std::span<const box> rargs) {
  return invoke_std(rargs, boxd::Llowercase);
}

box LmatchSubString(std::span<const box> rargs) {
  return invoke_std(rargs, boxd::LmatchSubString);
}

box Lsubstring(std::span<const box> rargs) {
  return invoke_std(rargs, boxd::Lsubstring);
}

box Lregexp(std::span<const box> rargs) {
  return invoke_std(rargs, boxd::Lregexp);
}

box LregexpMatch(std::span<const box> rargs) {
  return invoke_std(rargs, boxd::LregexpMatch);
}

box Lclone(std::span<const box> rargs) {
  return invoke_std(rargs, boxd::Lclone);
}

box LstringInt(std::span<const box> rargs) {
  return invoke_std(rargs, boxd::LstringInt);
}

box Lhash(std::span<const box> rargs) {
  return invoke_std(rargs, boxd::Lhash);
}

box LflatCompare(std::span<const box> rargs) {
  return invoke_std(rargs, boxd::LflatCompare);
}

box Lcompare(std::span<const box> rargs) {
  return invoke_std(rargs, boxd::Lcompare);
}

box Belem(std::span<const box> rargs) {
  return invoke_std(rargs, boxd::Belem);
}

box LmakeArray(std::span<const box> rargs) {
  return invoke_std(rargs, boxd::LmakeArray);
}

box LmakeString(std::span<const box> rargs) {
  return invoke_std(rargs, boxd::LmakeString);
}

box Bstring(std::span<const box> rargs) {
  return invoke_std(rargs, boxd::Bstring);
}

box Lstringcat(std::span<const box> rargs) {
  return invoke_std(rargs, boxd::Lstringcat);
}

box Lstring(std::span<const box> rargs) {
  return invoke_std(rargs, boxd::Lstring);
}

box Bclosure(std::span<const box> rargs) {
  return invoke_std(rargs, boxd::Bclosure);
}

box Barray(std::span<const box> rargs) {
  return invoke_std(rargs, boxd::Barray);
}

box Bsexp(std::span<const box> rargs) {
  return invoke_std(rargs, boxd::Bsexp);
}

box Btag(std::span<const box> rargs) {
  return invoke_std(rargs, boxd::Btag);
}

box Barray_patt(std::span<const box> rargs) {
  return invoke_std(rargs, boxd::Barray_patt);
}

box Bstring_patt(std::span<const box> rargs) {
  return invoke_std(rargs, boxd::Bstring_patt);
}

box Bclosure_tag_patt(std::span<const box> rargs) {
  return invoke_std(rargs, boxd::Bclosure_tag_patt);
}

box Bboxed_patt(std::span<const box> rargs) {
  return invoke_std(rargs, boxd::Bboxed_patt);
}

box Bunboxed_patt(std::span<const box> rargs) {
  return invoke_std(rargs, boxd::Bunboxed_patt);
}

box Barray_tag_patt(std::span<const box> rargs) {
  return invoke_std(rargs, boxd::Barray_tag_patt);
}

box Bstring_tag_patt(std::span<const box> rargs) {
  return invoke_std(rargs, boxd::Bstring_tag_patt);
}

box Bsexp_tag_patt(std::span<const box> rargs) {
  return invoke_std(rargs, boxd::Bsexp_tag_patt);
}

box Bsta(std::span<const box> rargs) {
  return invoke_std(rargs, boxd::Bsta);
}

void Bmatch_failure(std::span<const box> rargs) {
  invoke_std(rargs, boxd::Bmatch_failure);
}

box Li__Infix_4343(std::span<const box> rargs) {
  return invoke_std(rargs, boxd::Li__Infix_4343);
}

box LgetEnv(std::span<const box> rargs) {
  return invoke_std(rargs, boxd::LgetEnv);
}

box Lsystem(std::span<const box> rargs) {
  return invoke_std(rargs, boxd::Lsystem);
}

box Bsprintf(std::span<const box> rargs) {
  return invoke_std(rargs, boxd::Bsprintf);
}

void Bprintf(std::span<const box> rargs) {
  invoke_std(rargs, boxd::Bprintf);
}

void Bfprintf(std::span<const box> rargs) {
  invoke_std(rargs, boxd::Bfprintf);
}

box Lfopen(std::span<const box> rargs) {
  return invoke_std(rargs, boxd::Lfopen);
}

void Lfclose(std::span<const box> rargs) {
  invoke_std(rargs, boxd::Lfclose);
}

box LreadLine(std::span<const box> rargs) {
  return invoke_std(rargs, boxd::LreadLine);
}

box Lfread(std::span<const box> rargs) {
  return invoke_std(rargs, boxd::Lfread);
}

void Lfwrite(std::span<const box> rargs) {
  invoke_std(rargs, boxd::Lfwrite);
}

box Lfexists(std::span<const box> rargs) {
  return invoke_std(rargs, boxd::Lfexists);
}

box Lfst(std::span<const box> rargs) {
  return invoke_std(rargs, boxd::Lfst);
}

box Lsnd(std::span<const box> rargs) {
  return invoke_std(rargs, boxd::Lsnd);
}

box Lhd(std::span<const box> rargs) {
  return invoke_std(rargs, boxd::Lhd);
}

box Ltl(std::span<const box> rargs) {
  return invoke_std(rargs, boxd::Ltl);
}

box Lread(std::span<const box> rargs) {
  return invoke_std(rargs, boxd::Lread);
}

box Lwrite(std::span<const box> rargs) {
  return invoke_std(rargs, boxd::Lwrite);
}

box Lrandom(std::span<const box> rargs) {
  return invoke_std(rargs, boxd::Lrandom);
}

box Ltime(std::span<const box> rargs) {
  return invoke_std(rargs, boxd::Ltime);
}

} // namespace span

} // namespace lamavm::interpreter
