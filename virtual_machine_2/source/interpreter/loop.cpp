#include "loop.hpp"

#include "bc/unit.hpp"
#include "core/assert.hpp"

extern "C" {
#include <contrib/runtime/gc.h>
}

#include <atomic>
#include <cstddef>

namespace lamavm::interpreter {

namespace {

std::byte* ip = nullptr; // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)
std::byte* sp = nullptr; // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)
std::byte* bp = nullptr; // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)

std::vector<std::byte> stack; // NOLINT

} // namespace

void load(const bc::unit_t& unit) {
  static std::atomic_bool is_loaded = false;
  LAMAVM_ASSERT(!is_loaded.exchange(true));

  (void)unit;
}

void run() {
  (void)ip, (void)sp, (void)bp;
}

} // namespace lamavm::interpreter
