#pragma once

#include <string_view>

namespace lamavm {

[[noreturn]] void emergency(const char* message);
[[noreturn]] void emergency(std::string_view message) noexcept;
[[noreturn]] void signal_emergency(std::string_view message) noexcept;

} // namespace lamavm
