#include "args.hpp"
#include "bc/input.hpp"
#include "bc/lamac_ds.hpp"

#include <cstddef>
#include <exception>
#include <iostream>
#include <string_view>

namespace {

class listener final : public lamavm::bc::lamac_ds_listener {
public:
  using lamac_ds_listener::lamac_ds_listener;
  using lamac_ds_listener::operator();

  void operator()(std::size_t offset, std::string_view message) override {
    std::cerr << "offset " << offset << ": " << message << '\n';
  }
};

} // namespace

auto main(int argc, char **argv) -> int try {
  const auto arguments = lamavm::args::parse(argc, argv);
  if (!arguments) {
    return arguments.error();
  }

  if (arguments->ds) {
    listener output(std::cout);
    if (const auto e = lamavm::bc::read_file(std::cin, output); !e) {
      return 1;
    }
  }

  return 0;
} catch (const std::exception &e) {
  std::cerr << e.what() << '\n';
  return 1;
}
