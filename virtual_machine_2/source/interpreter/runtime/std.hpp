#pragma once

#include "box.hpp"

#include <span>
#include <string_view>
#include <unordered_map>
#include <variant>

namespace lamavm::interpreter {

using std_fn_some_t = box (*)(std::span<const box> rargs);
using std_fn_none_t = void (*)(std::span<const box> rargs);

struct std_fn_symbol {
  enum class tail_kind : std::uint8_t {
    none,
    vararg,
  };

  std::variant<std_fn_some_t, std_fn_none_t> fn;
  std::size_t head = 0;
  tail_kind tail = tail_kind::none;
};

namespace boxd {

[[noreturn]] void failure(box s, ...);
void Lassert(box f, box s, ...);                          // NOLINT
box LkindOf(box p);                                       // NOLINT
box LcompareTags(box p, box q);                           // NOLINT
box Ls__Infix_58(box args);                               // NOLINT
box Ls__Infix_3333(box p, box q);                         // NOLINT
box Ls__Infix_3838(box p, box q);                         // NOLINT
box Ls__Infix_6161(box p, box q);                         // NOLINT
box Ls__Infix_3361(box p, box q);                         // NOLINT
box Ls__Infix_6061(box p, box q);                         // NOLINT
box Ls__Infix_60(box p, box q);                           // NOLINT
box Ls__Infix_6261(box p, box q);                         // NOLINT
box Ls__Infix_62(box p, box q);                           // NOLINT
box Ls__Infix_43(box p, box q);                           // NOLINT
box Ls__Infix_45(box p, box q);                           // NOLINT
box Ls__Infix_42(box p, box q);                           // NOLINT
box Ls__Infix_47(box p, box q);                           // NOLINT
box Ls__Infix_37(box p, box q);                           // NOLINT
box Llength(box p);                                       // NOLINT
box LtagHash(box s);                                      // NOLINT
box Luppercase(box v);                                    // NOLINT
box Llowercase(box v);                                    // NOLINT
box LmatchSubString(box subj, box patt, box pos);         // NOLINT
box Lsubstring(box args);                                 // NOLINT
box Lregexp(box regexp);                                  // NOLINT
box LregexpMatch(box b, box s, box pos);                  // NOLINT
box Lclone(box args);                                     // NOLINT
box LstringInt(box b);                                    // NOLINT
box Lhash(box p);                                         // NOLINT
box LflatCompare(box p, box q);                           // NOLINT
box Lcompare(box p, box q);                               // NOLINT
box Belem(box p, box i);                                  // NOLINT
box LmakeArray(box length);                               // NOLINT
box LmakeString(box length);                              // NOLINT
box Bstring(box args);                                    // NOLINT
box Lstringcat(box args);                                 // NOLINT
box Lstring(box args);                                    // NOLINT
box Bclosure(box args, box bn);                           // NOLINT
box Barray(box args, box bn);                             // NOLINT
box Bsexp(box args, box bn);                              // NOLINT
box Btag(box d, box t, box n);                            // NOLINT
box Barray_patt(box d, box n);                            // NOLINT
box Bstring_patt(box x, box y);                           // NOLINT
box Bclosure_tag_patt(box x);                             // NOLINT
box Bboxed_patt(box x);                                   // NOLINT
box Bunboxed_patt(box x);                                 // NOLINT
box Barray_tag_patt(box x);                               // NOLINT
box Bstring_tag_patt(box x);                              // NOLINT
box Bsexp_tag_patt(box x);                                // NOLINT
box Bsta(box x, box i, box v);                            // NOLINT
void Bmatch_failure(box v, box fname, box line, box col); // NOLINT
box Li__Infix_4343(box args);                             // NOLINT
box LgetEnv(box var);                                     // NOLINT
box Lsystem(box cmd);                                     // NOLINT
box Bsprintf(box fmt, ...);                               // NOLINT
void Bprintf(box s, ...);                                 // NOLINT
void Bfprintf(box f, box s, ...);                         // NOLINT
box Lfopen(box f, box m);                                 // NOLINT
void Lfclose(box f);                                      // NOLINT
box LreadLine();                                          // NOLINT
box Lfread(box fname);                                    // NOLINT
void Lfwrite(box fname, box contents);                    // NOLINT
box Lfexists(box fname);                                  // NOLINT
box Lfst(box v);                                          // NOLINT
box Lsnd(box v);                                          // NOLINT
box Lhd(box v);                                           // NOLINT
box Ltl(box v);                                           // NOLINT
box Lread();                                              // NOLINT
box Lwrite(box n);                                        // NOLINT
box Lrandom(box n);                                       // NOLINT
box Ltime();                                              // NOLINT

} // namespace boxd

namespace span {

void failure(std::span<const box> rargs);
void Lassert(std::span<const box> rargs);          // NOLINT
box LkindOf(std::span<const box> rargs);           // NOLINT
box LcompareTags(std::span<const box> rargs);      // NOLINT
box Ls__Infix_58(std::span<const box> rargs);      // NOLINT
box Ls__Infix_3333(std::span<const box> rargs);    // NOLINT
box Ls__Infix_3838(std::span<const box> rargs);    // NOLINT
box Ls__Infix_6161(std::span<const box> rargs);    // NOLINT
box Ls__Infix_3361(std::span<const box> rargs);    // NOLINT
box Ls__Infix_6061(std::span<const box> rargs);    // NOLINT
box Ls__Infix_60(std::span<const box> rargs);      // NOLINT
box Ls__Infix_6261(std::span<const box> rargs);    // NOLINT
box Ls__Infix_62(std::span<const box> rargs);      // NOLINT
box Ls__Infix_43(std::span<const box> rargs);      // NOLINT
box Ls__Infix_45(std::span<const box> rargs);      // NOLINT
box Ls__Infix_42(std::span<const box> rargs);      // NOLINT
box Ls__Infix_47(std::span<const box> rargs);      // NOLINT
box Ls__Infix_37(std::span<const box> rargs);      // NOLINT
box Llength(std::span<const box> rargs);           // NOLINT
box LtagHash(std::span<const box> rargs);          // NOLINT
box Luppercase(std::span<const box> rargs);        // NOLINT
box Llowercase(std::span<const box> rargs);        // NOLINT
box LmatchSubString(std::span<const box> rargs);   // NOLINT
box Lsubstring(std::span<const box> rargs);        // NOLINT
box Lregexp(std::span<const box> rargs);           // NOLINT
box LregexpMatch(std::span<const box> rargs);      // NOLINT
box Lclone(std::span<const box> rargs);            // NOLINT
box LstringInt(std::span<const box> rargs);        // NOLINT
box Lhash(std::span<const box> rargs);             // NOLINT
box LflatCompare(std::span<const box> rargs);      // NOLINT
box Lcompare(std::span<const box> rargs);          // NOLINT
box Belem(std::span<const box> rargs);             // NOLINT
box LmakeArray(std::span<const box> rargs);        // NOLINT
box LmakeString(std::span<const box> rargs);       // NOLINT
box Bstring(std::span<const box> rargs);           // NOLINT
box Lstringcat(std::span<const box> rargs);        // NOLINT
box Lstring(std::span<const box> rargs);           // NOLINT
box Bclosure(std::span<const box> rargs);          // NOLINT
box Barray(std::span<const box> rargs);            // NOLINT
box Bsexp(std::span<const box> rargs);             // NOLINT
box Btag(std::span<const box> rargs);              // NOLINT
box Barray_patt(std::span<const box> rargs);       // NOLINT
box Bstring_patt(std::span<const box> rargs);      // NOLINT
box Bclosure_tag_patt(std::span<const box> rargs); // NOLINT
box Bboxed_patt(std::span<const box> rargs);       // NOLINT
box Bunboxed_patt(std::span<const box> rargs);     // NOLINT
box Barray_tag_patt(std::span<const box> rargs);   // NOLINT
box Bstring_tag_patt(std::span<const box> rargs);  // NOLINT
box Bsexp_tag_patt(std::span<const box> rargs);    // NOLINT
box Bsta(std::span<const box> rargs);              // NOLINT
void Bmatch_failure(std::span<const box> rargs);   // NOLINT
box Li__Infix_4343(std::span<const box> rargs);    // NOLINT
box LgetEnv(std::span<const box> rargs);           // NOLINT
box Lsystem(std::span<const box> rargs);           // NOLINT
box Bsprintf(std::span<const box> rargs);          // NOLINT
void Bprintf(std::span<const box> rargs);          // NOLINT
void Bfprintf(std::span<const box> rargs);         // NOLINT
box Lfopen(std::span<const box> rargs);            // NOLINT
void Lfclose(std::span<const box> rargs);          // NOLINT
box LreadLine(std::span<const box> rargs);         // NOLINT
box Lfread(std::span<const box> rargs);            // NOLINT
void Lfwrite(std::span<const box> rargs);          // NOLINT
box Lfexists(std::span<const box> rargs);          // NOLINT
box Lfst(std::span<const box> rargs);              // NOLINT
box Lsnd(std::span<const box> rargs);              // NOLINT
box Lhd(std::span<const box> rargs);               // NOLINT
box Ltl(std::span<const box> rargs);               // NOLINT
box Lread(std::span<const box> rargs);             // NOLINT
box Lwrite(std::span<const box> rargs);            // NOLINT
box Lrandom(std::span<const box> rargs);           // NOLINT
box Ltime(std::span<const box> rargs);             // NOLINT

} // namespace span

using std_fn_symbol::tail_kind::vararg;

// NOLINTNEXTLINE(bugprone-throwing-static-initialization)
const std::unordered_map<std::string_view, std_fn_symbol> std_symbols = {
  {"assert", {span::Lassert, 2U, vararg}},         // NOLINT
  {"getEnv", {span::LgetEnv, 1U}},                 // NOLINT
  {"system", {span::Lsystem, 1U}},                 // NOLINT
  {"stringInt", {span::LstringInt, 1U}},           // NOLINT
  {"makeArray", {span::LmakeArray, 1U}},           // NOLINT
  {"string", {span::Lstring, 1U}},                 // NOLINT
  {"length", {span::Llength, 1U}},                 // NOLINT
  {"clone", {span::Lclone, 1U}},                   // NOLINT
  {"hash", {span::Lhash, 1U}},                     // NOLINT
  {"fst", {span::Lfst, 1U}},                       // NOLINT
  {"snd", {span::Lsnd, 1U}},                       // NOLINT
  {"hd", {span::Lhd, 1U}},                         // NOLINT
  {"tl", {span::Ltl, 1U}},                         // NOLINT
  {"readLine", {span::LreadLine, 0U}},             // NOLINT
  {"stringcat", {span::Lstringcat, 1U}},           // NOLINT
  {"matchSubString", {span::LmatchSubString, 3U}}, // NOLINT
  {"substring", {span::Lsubstring, 1U}},           // NOLINT
  {"regexp", {span::Lregexp, 1U}},                 // NOLINT
  {"regexpMatch", {span::LregexpMatch, 3U}},       // NOLINT
  {"sprintf", {span::Bsprintf, 1U, vararg}},       // NOLINT
  {"makeString", {span::LmakeString, 1U}},         // NOLINT
  {"printf", {span::Bprintf, 1U, vararg}},         // NOLINT
  {"fprintf", {span::Bfprintf, 2U, vararg}},       // NOLINT
  {"fopen", {span::Lfopen, 2U}},                   // NOLINT
  {"fclose", {span::Lfclose, 1U}},                 // NOLINT
  {"fread", {span::Lfread, 1U}},                   // NOLINT
  {"fwrite", {span::Lfwrite, 2U}},                 // NOLINT
  {"fexists", {span::Lfexists, 1U}},               // NOLINT
  {"failure", {span::failure, 1U, vararg}},        // NOLINT
  {"read", {span::Lread, 0U}},                     // NOLINT
  {"write", {span::Lwrite, 1U}},                   // NOLINT
  {"compare", {span::Lcompare, 2U}},               // NOLINT
  {"random", {span::Lrandom, 1U}},                 // NOLINT
  {"time", {span::Ltime, 0U}},                     // NOLINT
  {"kindOf", {span::LkindOf, 1U}},                 // NOLINT
  {"compareTags", {span::LcompareTags, 2U}},       // NOLINT
  {"flatCompare", {span::LflatCompare, 2U}},       // NOLINT
  {"tagHash", {span::LtagHash, 1U}},               // NOLINT
  {"uppercase", {span::Luppercase, 1U}},           // NOLINT
  {"lowercase", {span::Llowercase, 1U}},           // NOLINT
};

} // namespace lamavm::interpreter
