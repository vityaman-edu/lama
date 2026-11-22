#pragma once

#include "log.hpp" // IWYU pragma: keep

namespace lamavm {

enum class verify_kind : std::uint8_t { hard, soft };

using verify_kind::hard;
using verify_kind::soft;

#ifdef NDEBUG
constexpr bool is_ndebug = true;
#else
constexpr bool is_ndebug = false;
#endif

} // namespace lamavm

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define LAMAVM_VERIFY_C(kind, c, m)                                                                \
  do { /* NOLINT(cppcoreguidelines-avoid-do-while) */                                              \
    if constexpr (!::lamavm::is_ndebug || (kind) == ::lamavm::hard) {                              \
      if (!(c)) {                                                                                  \
        ::lamavm::fatalf("{}:{}:1: unexepectedly {}", __FILE__, __LINE__, (m));                    \
      }                                                                                            \
    }                                                                                              \
  } while (false)

#define LAMAVM_VERIFY(kind, c) LAMAVM_VERIFY_C(kind, c, #c " is false")

#define LAMAVM_VERIFY_B(kind, lhs, op, rhs)                                                        \
  do { /* NOLINT(cppcoreguidelines-avoid-do-while) */                                              \
    if constexpr (!::lamavm::is_ndebug || (kind) == ::lamavm::hard) {                              \
      auto&& lamavm_verify_lhs = (lhs);                                                            \
      auto&& lamavm_verify_rhs = (rhs);                                                            \
      LAMAVM_VERIFY_C(                                                                             \
        kind,                                                                                      \
        lamavm_verify_lhs op lamavm_verify_rhs,                                                    \
        std::format(                                                                               \
          "{} {} {} ({} {} {}) is false",                                                          \
          #lhs,                                                                                    \
          #op,                                                                                     \
          #rhs,                                                                                    \
          lamavm_verify_lhs,                                                                       \
          #op,                                                                                     \
          lamavm_verify_rhs));                                                                     \
    }                                                                                              \
  } while (false)

#define LAMAVM_VERIFY_BB(kind, a, ab, b, bc, c)                                                    \
  if constexpr (!::lamavm::is_ndebug || (kind) == ::lamavm::hard) {                                \
    LAMAVM_VERIFY_B(kind, a, ab, b);                                                               \
    LAMAVM_VERIFY_B(kind, b, bc, c);                                                               \
  } else                                                                                           \
    static_cast<void>(0)

#define LAMAVM_EXPECT(c) LAMAVM_VERIFY(::lamavm::hard, c)

#define LAMAVM_EXPECT_C(c, m) LAMAVM_VERIFY_C(::lamavm::hard, c, m)

#define LAMAVM_EXPECT_B(lhs, op, rhs) LAMAVM_VERIFY_B(::lamavm::hard, lhs, op, rhs)

#define LAMAVM_EXPECT_BB(a, ab, b, bc, c) LAMAVM_VERIFY_BB(::lamavm::hard, a, ab, b, bc, c)

#define LAMAVM_ASSERT(c) LAMAVM_VERIFY(::lamavm::soft, c)

#define LAMAVM_ASSERT_C(c, m) LAMAVM_VERIFY_C(::lamavm::soft, c, m)

#define LAMAVM_ASSERT_B(lhs, op, rhs) LAMAVM_VERIFY_B(::lamavm::soft, lhs, op, rhs)

#define LAMAVM_ASSERT_BB(a, ab, b, bc, c) LAMAVM_VERIFY_BB(::lamavm::soft, a, ab, b, bc, c)

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define LAMAVM_UNEXPECTEDLY(s, ...) LAMAVM_EXPECT_C(false, std::format(s, __VA_ARGS__))
