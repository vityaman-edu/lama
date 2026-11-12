#include "args.hpp"

#include "bc/unit.hpp"
#include "core/log.hpp"

#include <exception>
#include <ios>

namespace lamavm {

namespace {

// NOLINTNEXTLINE(modernize-avoid-c-arrays)
void main(int argc, char* argv[]) try {
  args const args = args::parse(argc, argv);

  bc::unit_t const unit = bc::parse_unit(args.bc_path);
  (void)unit;

  fatalf("not implemented {}", "yet");
} catch (const std::ios_base::failure& e) {
  fatalf("io error: {}", e.code().message());
} catch (const std::exception& e) {
  fatalf("unexpected exception: {}", e.what());
} catch (...) {
  fatalf("unexpected exception");
}

} // namespace

} // namespace lamavm

int main(int argc, char* argv[]) {
  lamavm::main(argc, argv);
  return 0;
}
