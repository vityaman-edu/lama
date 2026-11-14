#pragma once

#include "log.hpp" // IWYU pragma: keep

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define LAMAVM_EXPECT_C(c, m)                                                                      \
  do { /* NOLINT(cppcoreguidelines-avoid-do-while) */                                              \
    if (!(c)) {                                                                                    \
      ::lamavm::fatalf("{}:{}:1: unexepectedly {}", __FILE__, __LINE__, (m));                      \
    }                                                                                              \
  } while (false)

#define LAMAVM_EXPECT(c) LAMAVM_EXPECT_C(c, #c " is false")

#define LAMAVM_EXPECT_B(lhs, op, rhs)                                                              \
  do { /* NOLINT(cppcoreguidelines-avoid-do-while) */                                              \
    auto&& lamavm_expect_lhs = (lhs);                                                              \
    auto&& lamavm_expect_rhs = (rhs);                                                              \
    LAMAVM_EXPECT_C(                                                                               \
      lamavm_expect_lhs op lamavm_expect_rhs,                                                      \
      std::format(                                                                                 \
        "{} {} {} ({} {} {})",                                                                     \
        #lhs,                                                                                      \
        #op,                                                                                       \
        #rhs,                                                                                      \
        lamavm_expect_lhs,                                                                         \
        #op,                                                                                       \
        lamavm_expect_rhs));                                                                       \
  } while (false)

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define LAMAVM_EXPECT_BB(a, ab, b, bc, c)                                                          \
  LAMAVM_EXPECT_B(a, ab, b);                                                                       \
  LAMAVM_EXPECT_B(b, bc, c)

#ifdef NDEBUG

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define LAMAVM_ASSERT_C(c, m)                                                                      \
  (void)(c);                                                                                       \
  (void)(m) // NOLINT(cppcoreguidelines-macro-usage)

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define LAMAVM_ASSERT(c) (void)(c)

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define LAMAVM_ASSERT_B(lhs, op, rhs)                                                              \
  (void)(lhs);                                                                                     \
  (void)(rhs)

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define LAMAVM_ASSERT_BB(a, ab, b, bc, c)                                                          \
  (void)(a);                                                                                       \
  (void)(b);                                                                                       \
  (void)(c)

#else

#define LAMAVM_ASSERT_C(c, m) LAMAVM_EXPECT_C(c, m)

#define LAMAVM_ASSERT(c) LAMAVM_EXPECT(c)

#define LAMAVM_ASSERT_B(lhs, op, rhs) LAMAVM_EXPECT_B(lhs, op, rhs)

#define LAMAVM_ASSERT_BB(lhs, op, rhs) LAMAVM_EXPECT_BB(lhs, op, rhs)

#endif
