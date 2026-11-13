#pragma once

#include <format>      // IWYU pragma: keep
#include <string_view> // IWYU pragma: keep

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define LAMAVM_FORMAT_DECLARE(t, x, ctx)                                                           \
  template <>                                                                                      \
  struct std::formatter<t> : std::formatter<std::string_view> {                                    \
    std::format_context::iterator format(t x, std::format_context&(ctx)) const;                    \
  }

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define LAMAVM_FORMAT_DEFINE(t, x, ctx)                                                            \
  std::format_context::iterator std::formatter<t>::format(t x, std::format_context&(ctx)) const

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define LAMAVM_FORMAT_DERIVE(ctx, p, ...)                                                          \
  std::formatter<std::string_view>::format(std::format((p)__VA_OPT__(, ) __VA_ARGS__), (ctx))
