#include "args.hpp"
#include "bc/disassembly/legacy.hpp"
#include "bc/disassembly/modern.hpp"
#include "bc/file_listener.hpp"
#include "bc/file_listener_nop.hpp"
#include "bc/file_listener_seq.hpp"
#include "bc/file_reader.hpp"
#include "interpreter/boot.hpp"

#include <exception>
#include <iostream>
#include <memory>
#include <utility>

auto main(int argc, char **argv) -> int try {
  const auto arguments = lamavm::args::parse(argc, argv);
  if (!arguments) {
    std::cerr << arguments.error() << '\n';
    return 2;
  }

  if (arguments->is_help) {
    return 0;
  }

  std::unique_ptr<lamavm::bc::file_listener> listener;
  switch (arguments->disassembly) {
  case lamavm::disassembly_kind::none:
    listener = std::make_unique<lamavm::bc::file_listener_nop>();
    break;
  case lamavm::disassembly_kind::legacy:
    listener = std::make_unique<lamavm::bc::disassembly::legacy>(std::cout);
    break;
  case lamavm::disassembly_kind::modern:
    listener = std::make_unique<lamavm::bc::disassembly::modern>(std::cout);
    break;
  }

  lamavm::bc::file_listener_seq seq(
    std::move(listener),
    std::make_unique<lamavm::interpreter::boot>());

  if (const auto e = lamavm::bc::read_file(std::cin, seq); !e) {
    for (const auto &message : e.error()) {
      std::cerr << message << '\n';
    }

    return 1;
  }

  return 0;
} catch (const std::exception &e) {
  std::cerr << e.what() << '\n';
  return 1;
}
