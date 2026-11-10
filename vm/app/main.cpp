#include "args.hpp"
#include "bc/disassembly/legacy.hpp"
#include "bc/disassembly/modern.hpp"
#include "bc/file_reader.hpp"

#include <exception>
#include <iostream>

auto main(int argc, char **argv) -> int try {
  const auto arguments = lamavm::args::parse(argc, argv);
  if (!arguments) {
    std::cerr << arguments.error() << '\n';
    return 2;
  }

  if (arguments->is_help) {
    return 0;
  }

  switch (arguments->disassembly) {
  case lamavm::disassembly_kind::none:
    break;
  case lamavm::disassembly_kind::legacy: {
    lamavm::bc::disassembly::legacy listener(std::cout, std::cerr);
    if (const auto e = lamavm::bc::read_file(std::cin, listener); !e) {
      return 1;
    }

    break;
  }
  case lamavm::disassembly_kind::modern: {
    lamavm::bc::disassembly::modern listener(std::cout, std::cerr);
    if (const auto e = lamavm::bc::read_file(std::cin, listener); !e) {
      return 1;
    }

    break;
  }
  }

  return 0;
} catch (const std::exception &e) {
  std::cerr << e.what() << '\n';
  return 1;
}
