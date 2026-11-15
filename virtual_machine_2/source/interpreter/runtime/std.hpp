#pragma once

#include "box.hpp"

namespace lamavm::interpreter {

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

} // namespace lamavm::interpreter
