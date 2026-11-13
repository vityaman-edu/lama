#pragma once

#include <cstdint>

namespace lamavm::interpreter {

extern "C" {
std::int64_t Ls__Infix_43(std::int64_t x, std::int64_t y);   // NOLINT
std::int64_t Ls__Infix_45(std::int64_t x, std::int64_t y);   // NOLINT
std::int64_t Ls__Infix_42(std::int64_t x, std::int64_t y);   // NOLINT
std::int64_t Ls__Infix_47(std::int64_t x, std::int64_t y);   // NOLINT
std::int64_t Ls__Infix_37(std::int64_t x, std::int64_t y);   // NOLINT
std::int64_t Ls__Infix_60(std::int64_t x, std::int64_t y);   // NOLINT
std::int64_t Ls__Infix_6061(std::int64_t x, std::int64_t y); // NOLINT
std::int64_t Ls__Infix_62(std::int64_t x, std::int64_t y);   // NOLINT
std::int64_t Ls__Infix_6261(std::int64_t x, std::int64_t y); // NOLINT
std::int64_t Ls__Infix_6161(std::int64_t x, std::int64_t y); // NOLINT
std::int64_t Ls__Infix_3361(std::int64_t x, std::int64_t y); // NOLINT
std::int64_t Ls__Infix_3838(std::int64_t x, std::int64_t y); // NOLINT
std::int64_t Ls__Infix_3333(std::int64_t x, std::int64_t y); // NOLINT
}

bool is_unboxed(std::int64_t x);

std::int64_t box(std::int64_t x);

std::int64_t unbox(std::int64_t x);

} // namespace lamavm::interpreter
