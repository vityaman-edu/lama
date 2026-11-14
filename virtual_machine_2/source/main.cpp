#include "args.hpp"

#include "bc/unit.hpp"
#include "bc/units.hpp"
#include "core/assert.hpp"
#include "core/log.hpp"
#include "interpreter/naive/loop.hpp"

#include <exception>
#include <ios>
#include <iostream>
#include <ostream>

namespace lamavm {

namespace {

// NOLINTNEXTLINE(modernize-avoid-c-arrays)
void main(int argc, char* argv[]) try {
  args const args = args::parse(argc, argv);

  bc::units units = bc::units::from(args.bc_path);
  LAMAVM_EXPECT_B(units.initialization_order.size(), ==, 1U);

  bc::unit main = units.initialization_order.front()->second;

  interpreter::run(main);
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
