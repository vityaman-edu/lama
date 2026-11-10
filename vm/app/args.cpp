#include "args.hpp"

#include <cxxopts.hpp>

#include <expected>
#include <iostream>
#include <string>

namespace lamavm {

auto args::parse(int argc, char **argv) -> std::expected<args, std::string> {
  try {
    args parsed;
    std::string disassembly;
    cxxopts::Options options("lamavm", "Disassemble LaMa bytecode from standard input");
    options.add_options()(
      "disassembly",
      "Disassembly kind: none, legacy, modern",
      cxxopts::value(disassembly)->default_value("none"))(
      "h,help",
      "Print usage",
      cxxopts::value(parsed.is_help));

    const auto result = options.parse(argc, argv);
    if (!result.unmatched().empty()) {
      return std::unexpected("unexpected argument: " + result.unmatched().front());
    }

    if (parsed.is_help) {
      std::cout << options.help() << '\n';
      return parsed;
    }

    if (disassembly == "none") {
      parsed.disassembly = disassembly_kind::none;
    } else if (disassembly == "legacy") {
      parsed.disassembly = disassembly_kind::legacy;
    } else if (disassembly == "modern") {
      parsed.disassembly = disassembly_kind::modern;
    } else {
      return std::unexpected("invalid disassembly kind: " + disassembly);
    }

    return parsed;
  } catch (const cxxopts::exceptions::exception &e) {
    return std::unexpected(std::string(e.what()));
  }
}

} // namespace lamavm
