#pragma once

#include "log.hpp" // IWYU pragma: keep

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define LAMAVM_ASSERT_C(c, m)                                                                      \
  do { /* NOLINT(cppcoreguidelines-avoid-do-while) */                                              \
    if (!(c)) {                                                                                    \
      ::lamavm::fatalf("{}:{}: assertion failed: {}", __FILE__, __LINE__, #c, (m));                \
    }                                                                                              \
  } while (false)

#define LAMAVM_ASSERT(c) LAMAVM_ASSERT_C(c, #c " is false")

#define LAMAVM_ASSERT_EQ(lhs, rhs) LAMAVM_ASSERT_C((lhs) == (rhs), "(" #rhs ") != (" #lhs ")");

#define LAMAVM_ASSERT_LT(lhs, rhs) LAMAVM_ASSERT_C((lhs) < (rhs), "(" #rhs ") < (" #lhs ")");

#define LAMAVM_ASSERT_LE(lhs, rhs) LAMAVM_ASSERT_C((lhs) <= (rhs), "(" #rhs ") <= (" #lhs ")");
