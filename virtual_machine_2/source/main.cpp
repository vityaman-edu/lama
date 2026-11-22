#include "args.hpp"

#include "bc/units.hpp"
#include "core/log.hpp"
#include "interpreter/run.hpp"

#include <cstdlib>
#include <cstring>
#include <exception>
#include <ios>
#include <iostream>
#include <new>
#include <ostream>
#include <utility>

#include <unistd.h>

namespace lamavm {

namespace {

// NOLINTNEXTLINE(modernize-avoid-c-arrays)
void main(int argc, char* argv[]) try {
  const args args = args::parse(argc, argv);
  bc::units units = bc::units::from(args.bc_path);
  interpreter::run(std::move(units));
} catch (const std::bad_alloc& e) {
  const char* message = e.what();
  std::ignore = ::write(STDERR_FILENO, message, std::strlen(message));
  std::ignore = ::write(STDERR_FILENO, "\n", 1);
  std::exit(EXIT_FAILURE); // NOLINT(concurrency-mt-unsafe)
} catch (const std::ios_base::failure& e) {
  fatalf("io error: {}", e.code().message());
} catch (const std::exception& e) {
  fatalf("unexpected exception: {}", e.what());
} catch (...) {
  fatalf("unexpected exception");
}

} // namespace

} // namespace lamavm

int main(int argc, char* argv[]) try {
  lamavm::main(argc, argv);
  return 0;
} catch (...) {
  std::cerr << "unexpected exception" << '\n' << std::flush;
}
