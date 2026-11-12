#include "args.hpp"

#include "core/log.hpp"

#include <cxxopts.hpp>

namespace lamavm {

args args::parse(int argc, char* argv[]) try {
  cxxopts::Options options("lamavm2", "LaMa virtual machine");
  options.add_options()("bytecode", "Bytecode path", cxxopts::value<std::string>());
  options.parse_positional({"bytecode"});

  const auto result = options.parse(argc, argv);
  if (result.count("bytecode") != 1 || !result.unmatched().empty()) {
    fatalf("expected exactly one positional argument: bytecode path");
  }

  return {.bc_path = result["bytecode"].as<std::string>()};
} catch (const cxxopts::exceptions::exception& error) {
  fatalf("bad args: {}", error.what());
}

} // namespace lamavm
