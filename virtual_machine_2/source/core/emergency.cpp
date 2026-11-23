#include "emergency.hpp"

#include <cstdlib>
#include <cstring>
#include <string_view>
#include <tuple>

#include <unistd.h>

namespace lamavm {

namespace {

void write_message(std::string_view message) noexcept {
  std::ignore = ::write(STDERR_FILENO, message.data(), message.size());
  std::ignore = ::write(STDERR_FILENO, "\n", 1);
}

} // namespace

void emergency(const char* message) {
  emergency({message, std::strlen(message)});
}

void emergency(std::string_view message) noexcept {
  write_message(message);
  std::exit(EXIT_FAILURE); // NOLINT(concurrency-mt-unsafe)
}

void signal_emergency(std::string_view message) noexcept {
  write_message(message);
  std::exit(EXIT_FAILURE); // NOLINT(concurrency-mt-unsafe)
}

} // namespace lamavm
