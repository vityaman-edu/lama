#include "code_reader.hpp"

#include "code_listener.hpp"
#include "code_reader.detail.hpp"

#include <cstddef>
#include <expected>
#include <span>
#include <string>
#include <utility>
#include <vector>

namespace lamavm::bc {

auto read_ops(std::span<const std::byte> code, code_listener &listener)
  -> std::expected<void, std::vector<std::string>> {
  if (auto result = detail::code_reader<code_listener, false>(code, listener).read_ops(); !result) {
    return std::unexpected(std::vector<std::string>{std::move(result.error())});
  }

  return {};
}

} // namespace lamavm::bc
