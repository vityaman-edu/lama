#pragma once

#include <cstddef>

namespace lamavm {

template <typename T>
struct function_traits;

template <typename R, typename... T>
struct function_traits<R (*)(T...)> {
  using return_type = R;

  static constexpr std::size_t arity = sizeof...(T);
  static constexpr bool is_variadic = false;
};

template <typename R, typename... T>
struct function_traits<R (*)(T..., ...)> : function_traits<R (*)(T...)> {
  static constexpr bool is_variadic = true;
};

} // namespace lamavm
