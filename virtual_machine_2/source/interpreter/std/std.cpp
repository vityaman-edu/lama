#include "std.hpp"

#include "interpreter/runtime/box.hpp"

#include "core/assert.hpp"
#include "core/log.hpp"

#include <contrib/libffi/ffi.h>
#include <contrib/runtime/runtime_common.h>

#include <algorithm>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <span>
#include <vector>

namespace lamavm::interpreter {

namespace cc {

extern "C" {

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
void* Belem(void* p, aint i);                                   // NOLINT
void* Bstring(aint* args);                                      // NOLINT
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
}

} // namespace cc

namespace detail {

box invoke_std(std::span<const box> rargs, box (*f)()) {
  return invoke_std_impl(rargs, f);
}

box invoke_array_std(std::span<box> rargs, box (*f)(box)) {
  std::ranges::reverse(rargs);
  return f(box(rargs.data()));
}

box invoke_variadic_std(
  std::span<const box> rargs,
  void (*f)(),
  std::size_t head,
  bool is_void_result) {
  LAMAVM_EXPECT(rargs.size() >= head);
  LAMAVM_EXPECT(rargs.size() <= std::numeric_limits<unsigned int>::max());

  // T O D O: so each vararg call is super-costly?!
  std::vector<std::int64_t> ints(rargs.size());
  std::vector<void*> ptrs(rargs.size());
  std::vector<void*> args(rargs.size());
  std::vector<ffi_type*> ts(rargs.size());

  for (std::size_t i = 0; i < head; ++i) {
    const auto arg = rargs[rargs.size() - 1 - i];

    ptrs[i] = std::bit_cast<void*>(arg);
    ts[i] = &ffi_type_pointer;
    args[i] = static_cast<void*>(&ptrs[i]);
  }

  for (std::size_t i = head; i < rargs.size(); ++i) {
    const auto arg = rargs[rargs.size() - 1 - i];

    if (arg.is_ptr()) {
      ptrs[i] = static_cast<void*>(arg);
      ts[i] = &ffi_type_pointer;
      args[i] = static_cast<void*>(&ptrs[i]);
    } else {
      ints[i] = static_cast<std::int64_t>(arg);
      ts[i] = &ffi_type_sint64;
      args[i] = &ints[i];
    }
  }

  ffi_cif cif;
  const auto status = ffi_prep_cif_var(
    &cif,
    FFI_DEFAULT_ABI,
    static_cast<unsigned int>(head),
    static_cast<unsigned int>(rargs.size()),
    is_void_result ? &ffi_type_void : &ffi_type_pointer,
    ts.data());

  if (status != FFI_OK) {
    fatalf("FFI prep failed: status={}", static_cast<int>(status));
  }

  const void* result = nullptr;
  ffi_call(&cif, f, is_void_result ? nullptr : static_cast<void*>(&result), args.data());
  return is_void_result ? box{} : std::bit_cast<box>(result);
}

} // namespace detail

namespace boxd {

box Ls__Infix_58(box args) {
  return detail::bridge<cc::Ls__Infix_58>(args);
}

box Ls__Infix_3333(box p, box q) {
  return detail::bridge<cc::Ls__Infix_3333>(p, q);
}

box Ls__Infix_3838(box p, box q) {
  return detail::bridge<cc::Ls__Infix_3838>(p, q);
}

box Ls__Infix_6161(box p, box q) {
  return detail::bridge<cc::Ls__Infix_6161>(p, q);
}

box Ls__Infix_3361(box p, box q) {
  return detail::bridge<cc::Ls__Infix_3361>(p, q);
}

box Ls__Infix_6061(box p, box q) {
  return detail::bridge<cc::Ls__Infix_6061>(p, q);
}

box Ls__Infix_60(box p, box q) {
  return detail::bridge<cc::Ls__Infix_60>(p, q);
}

box Ls__Infix_6261(box p, box q) {
  return detail::bridge<cc::Ls__Infix_6261>(p, q);
}

box Ls__Infix_62(box p, box q) {
  return detail::bridge<cc::Ls__Infix_62>(p, q);
}

box Ls__Infix_43(box p, box q) {
  return detail::bridge<cc::Ls__Infix_43>(p, q);
}

box Ls__Infix_45(box p, box q) {
  return detail::bridge<cc::Ls__Infix_45>(p, q);
}

box Ls__Infix_42(box p, box q) {
  return detail::bridge<cc::Ls__Infix_42>(p, q);
}

box Ls__Infix_47(box p, box q) {
  return detail::bridge<cc::Ls__Infix_47>(p, q);
}

box Ls__Infix_37(box p, box q) {
  return detail::bridge<cc::Ls__Infix_37>(p, q);
}

box Belem(box p, box i) {
  return detail::bridge<cc::Belem>(p, i);
}

box Bstring(box args) {
  return detail::bridge<cc::Bstring>(args);
}

box Bclosure(box args, box bn) {
  return detail::bridge<cc::Bclosure>(args, bn);
}

box Barray(box args, box bn) {
  return detail::bridge<cc::Barray>(args, bn);
}

box Bsexp(box args, box bn) {
  return detail::bridge<cc::Bsexp>(args, bn);
}

box Btag(box d, box t, box n) {
  return detail::bridge<cc::Btag>(d, t, n);
}

box Barray_patt(box d, box n) {
  return detail::bridge<cc::Barray_patt>(d, n);
}

box Bstring_patt(box x, box y) {
  return detail::bridge<cc::Bstring_patt>(x, y);
}

box Bclosure_tag_patt(box x) {
  return detail::bridge<cc::Bclosure_tag_patt>(x);
}

box Bboxed_patt(box x) {
  return detail::bridge<cc::Bboxed_patt>(x);
}

box Bunboxed_patt(box x) {
  return detail::bridge<cc::Bunboxed_patt>(x);
}

box Barray_tag_patt(box x) {
  return detail::bridge<cc::Barray_tag_patt>(x);
}

box Bstring_tag_patt(box x) {
  return detail::bridge<cc::Bstring_tag_patt>(x);
}

box Bsexp_tag_patt(box x) {
  return detail::bridge<cc::Bsexp_tag_patt>(x);
}

box Bsta(box x, box i, box v) {
  return detail::bridge<cc::Bsta>(x, i, v);
}

void Bmatch_failure(box v, box fname, box line, box col) {
  detail::bridge<cc::Bmatch_failure>(v, fname, line, col);
}

box Li__Infix_4343(box args) {
  return detail::bridge<cc::Li__Infix_4343>(args);
}

} // namespace boxd

} // namespace lamavm::interpreter
