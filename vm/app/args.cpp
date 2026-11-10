#include "args.hpp"

#include <cxxopts.hpp>

#include <expected>
#include <iostream>

namespace lamavm {

auto args::parse(int argc, char **argv) -> std::expected<args, int> {
  try {
    args parsed;
    bool help = false;
    cxxopts::Options options("lamavm", "Disassemble LaMa bytecode from standard input");
    options.add_options()                                       //
      ("ds", "Disassemble bytecode", cxxopts::value(parsed.ds)) //
      ("h,help", "Print usage", cxxopts::value(help));

    const auto result = options.parse(argc, argv);
    if (!result.unmatched().empty()) {
      std::cerr << "Unexpected argument: " << result.unmatched().front() << '\n';
      return std::unexpected(2);
    }

    if (help) {
      std::cout << options.help() << '\n';
      return std::unexpected(0);
    }

    return parsed;
  } catch (const cxxopts::exceptions::exception &e) {
    std::cerr << e.what() << '\n';
    return std::unexpected(2);
  }
}

} // namespace lamavm
