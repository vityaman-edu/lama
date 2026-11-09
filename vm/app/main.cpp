#include "bc/input.hpp"
#include "bc/lamac_ds.hpp"

#include <cstddef>
#include <exception>
#include <iostream>
#include <span>
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

}

auto main(int argc, char **argv) -> int {
  const auto args = std::span(argv, static_cast<std::size_t>(argc));
  if (args.size() != 1 && (args.size() != 2 || std::string_view(args.back()) != "-ds")) {
    std::cerr << "Usage: lamavm [-ds]\n";
    return 2;
  }

  try {
    listener output(std::cout);
    if (const auto e = lamavm::bc::read_file(std::cin, output); !e) {
      return 1;
    }

    return 0;
  } catch (const std::exception &e) {
    std::cerr << e.what() << '\n';
    return 1;
  }
}
