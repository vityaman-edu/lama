#pragma once

#include <cstdint>

#ifndef __x86_64__
#error "LaMa runtime requires x86_64"
#endif

namespace lamavm::runtime {

static_assert(sizeof(void *) == sizeof(std::int64_t));

auto add(void *left, void *right) -> std::int64_t __asm__(LAMAVM_RUNTIME_PREFIX "Ls__Infix_43");

auto sub(void *left, void *right) -> std::int64_t __asm__(LAMAVM_RUNTIME_PREFIX "Ls__Infix_45");

auto mul(void *left, void *right) -> std::int64_t __asm__(LAMAVM_RUNTIME_PREFIX "Ls__Infix_42");

auto div(void *left, void *right) -> std::int64_t __asm__(LAMAVM_RUNTIME_PREFIX "Ls__Infix_47");

auto mod(void *left, void *right) -> std::int64_t __asm__(LAMAVM_RUNTIME_PREFIX "Ls__Infix_37");

auto lt(void *left, void *right) -> std::int64_t __asm__(LAMAVM_RUNTIME_PREFIX "Ls__Infix_60");

auto le(void *left, void *right) -> std::int64_t __asm__(LAMAVM_RUNTIME_PREFIX "Ls__Infix_6061");

auto gt(void *left, void *right) -> std::int64_t __asm__(LAMAVM_RUNTIME_PREFIX "Ls__Infix_62");

auto ge(void *left, void *right) -> std::int64_t __asm__(LAMAVM_RUNTIME_PREFIX "Ls__Infix_6261");

auto eq(void *left, void *right) -> std::int64_t __asm__(LAMAVM_RUNTIME_PREFIX "Ls__Infix_6161");

auto ne(void *left, void *right) -> std::int64_t __asm__(LAMAVM_RUNTIME_PREFIX "Ls__Infix_3361");

auto logical_and(void *left, void *right) -> std::int64_t
  __asm__(LAMAVM_RUNTIME_PREFIX "Ls__Infix_3838");

auto logical_or(void *left, void *right) -> std::int64_t
  __asm__(LAMAVM_RUNTIME_PREFIX "Ls__Infix_3333");

} // namespace lamavm::runtime
