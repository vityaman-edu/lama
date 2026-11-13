#include "args.hpp"

#include "bc/unit.hpp"
#include "core/log.hpp"
#include "interpreter/loop.hpp"

#include <exception>
#include <ios>
#include <iostream>
#include <ostream>
#include <utility>

namespace lamavm {

namespace {

// NOLINTNEXTLINE(modernize-avoid-c-arrays)
void main(int argc, char* argv[]) try {
  args const args = args::parse(argc, argv);
  bc::unit unit = bc::parse_unit(args.bc_path);
  interpreter::run(std::move(unit));
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
