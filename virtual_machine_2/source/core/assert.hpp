#pragma once

#include "log.hpp" // IWYU pragma: keep

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define LAMAVM_ASSERT_C(c, m)                                                                      \
  do { /* NOLINT(cppcoreguidelines-avoid-do-while) */                                              \
    if (!(c)) {                                                                                    \
      ::lamavm::fatalf("{}:{}: assertion failed: {}", __FILE__, __LINE__, (m));                    \
    }                                                                                              \
  } while (false)

#define LAMAVM_ASSERT(c) LAMAVM_ASSERT_C(c, #c " is false")

#define LAMAVM_ASSERT_B(lhs, op, rhs)                                                              \
  do { /* NOLINT(cppcoreguidelines-avoid-do-while) */                                              \
    auto&& lamavm_assert_lhs = (lhs);                                                              \
    auto&& lamavm_assert_rhs = (rhs);                                                              \
    LAMAVM_ASSERT_C(                                                                               \
      lamavm_assert_lhs op lamavm_assert_rhs,                                                      \
      std::format(                                                                                 \
        "({}) {} ({}), where lhs = ({}), rhs = ({})",                                              \
        #lhs,                                                                                      \
        #op,                                                                                       \
        #rhs,                                                                                      \
        lamavm_assert_lhs,                                                                         \
        lamavm_assert_rhs));                                                                       \
  } while (false)
