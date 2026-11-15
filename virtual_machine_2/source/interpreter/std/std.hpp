#pragma once

#include "interpreter/runtime/box.hpp"

#include "core/assert.hpp"
#include "core/functional.hpp"

#include <bit>
#include <cstddef>
#include <cstdint>
#include <span>
#include <string_view>
#include <type_traits>
#include <utility>

namespace lamavm::interpreter {

using std_fn_some_t = box (*)(std::span<box> rargs);

struct std_fn_symbol {
  enum class tail_kind : std::uint8_t {
    none,
    vararg,
  };

  std_fn_some_t fn = nullptr;
  std::int32_t head = 0;
  tail_kind tail = tail_kind::none;
};

namespace detail {

[[noreturn]] void unsupported_variadic(std::string_view name);

box invoke_array_std(std::span<box> rargs, box (*f)(box));

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

} // namespace detail

namespace boxd {

[[noreturn]] void failure(box s, ...);
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
box Belem(box p, box i);                                  // NOLINT
box Bstring(box args);                                    // NOLINT
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

} // namespace boxd

} // namespace lamavm::interpreter
