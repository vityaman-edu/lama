#include "loop.hpp"
#include "bc/unit.hpp"

#include <cstddef>

namespace lamavm {

namespace {

std::byte* ip = nullptr; // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)
std::byte* sp = nullptr; // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)
std::byte* bp = nullptr; // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)

} // namespace

void interpret(const bc::unit_t& unit) {
  (void)unit, (void)ip, (void)sp, (void)bp;
}

} // namespace lamavm
