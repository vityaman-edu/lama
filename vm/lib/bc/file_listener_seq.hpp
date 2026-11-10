#pragma once

#include "file_listener.hpp"
#include "layout.hpp"

#include <array>
#include <cstddef>
#include <expected>
#include <memory>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace lamavm::bc {

class file_listener_seq : public file_listener {
public:
  file_listener_seq(std::unique_ptr<file_listener> first, std::unique_ptr<file_listener> second);

  using file_listener::operator();

  void operator()(const header &op) override;
  void operator()(const import &op) override;
  void operator()(const public_symbol &op) override;
  void operator()(std::string_view strings) override;
  auto operator()(std::span<const std::byte> code)
    -> std::expected<void, std::vector<std::string>> override;

private:
  std::array<std::unique_ptr<file_listener>, 2> listeners;
};

} // namespace lamavm::bc
