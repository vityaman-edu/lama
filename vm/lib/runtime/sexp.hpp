#pragma once

#include <cstdint>

namespace lamavm::runtime {

auto sexp(std::int64_t *args, std::int64_t count) -> void *__asm__(LAMAVM_RUNTIME_PREFIX "Bsexp");

auto tag_hash(const char *tag) -> std::int64_t __asm__(LAMAVM_RUNTIME_PREFIX "LtagHash");

} // namespace lamavm::runtime
