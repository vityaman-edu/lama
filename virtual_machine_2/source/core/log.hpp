#pragma once

#include <cstdio>
#include <cstdlib>
#include <format>
#include <print>
#include <string_view>

namespace lamavm {

#ifdef TRACE
constexpr bool is_trace = true;
#else
constexpr bool is_trace = false;
#endif

template <class... Ts>
void tracef(std::string_view s, Ts... xs) {
  if constexpr (is_trace) {
    std::println(stderr, "{}", std::vformat(s, std::make_format_args(xs...)));
    static_cast<void>(std::fflush(stderr));
  }
}

template <class... Ts>
[[noreturn]] void fatalf(std::string_view s, Ts... xs) {
  std::println(stderr, "{}", std::vformat(s, std::make_format_args(xs...)));
  static_cast<void>(std::fflush(stderr));
  std::exit(EXIT_FAILURE); // NOLINT(concurrency-mt-unsafe)
}

} // namespace lamavm
