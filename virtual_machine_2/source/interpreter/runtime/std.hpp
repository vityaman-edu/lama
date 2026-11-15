#pragma once

#include "box.hpp"

namespace lamavm::interpreter {

/* pure  */ box Ls__Infix_43(box x, box y);   // NOLINT
/* pure  */ box Ls__Infix_45(box x, box y);   // NOLINT
/* pure  */ box Ls__Infix_42(box x, box y);   // NOLINT
/* pure  */ box Ls__Infix_47(box x, box y);   // NOLINT
/* pure  */ box Ls__Infix_37(box x, box y);   // NOLINT
/* pure  */ box Ls__Infix_60(box x, box y);   // NOLINT
/* pure  */ box Ls__Infix_6061(box x, box y); // NOLINT
/* pure  */ box Ls__Infix_62(box x, box y);   // NOLINT
/* pure  */ box Ls__Infix_6261(box x, box y); // NOLINT
/* pure  */ box Ls__Infix_6161(box x, box y); // NOLINT
/* pure  */ box Ls__Infix_3361(box x, box y); // NOLINT
/* pure  */ box Ls__Infix_3838(box x, box y); // NOLINT
/* pure  */ box Ls__Infix_3333(box x, box y); // NOLINT
/* alloc */ box Bstring(box x);               // NOLINT
/* pure  */ box LtagHash(box x);              // NOLINT
/* alloc */ box Bsexp(box args, box bn);      // NOLINT
/* pure  */ box Btag(box d, box t, box n);    // NOLINT
/* pure  */ box Belem(box p, box i);          // NOLINT
/* alloc */ box Barray(box args, box bn);     // NOLINT
/* pure  */ box Barray_patt(box d, box n);    // NOLINT
/* alloc */ box Bclosure(box args, box bn);   // NOLINT
/* pure  */ box Bstring_patt(box x, box y);   // NOLINT
/* pure  */ box Bstring_tag_patt(box x);      // NOLINT
/* pure  */ box Barray_tag_patt(box x);       // NOLINT
/* pure  */ box Bsexp_tag_patt(box x);        // NOLINT
/* pure  */ box Bboxed_patt(box x);           // NOLINT
/* pure  */ box Bunboxed_patt(box x);         // NOLINT
/* pure  */ box Bclosure_tag_patt(box x);     // NOLINT

void Bmatch_failure(box v, box fname, box line, box col); // NOLINT

} // namespace lamavm::interpreter
