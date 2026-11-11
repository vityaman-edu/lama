#pragma once

#include "code_listener.hpp"
#include "code_reader.detail.hpp"
#include "core/exception.hpp"

#include <cstddef>
#include <expected>
#include <span>
#include <vector>

namespace lamavm::bc {

template <typename Listener, bool unsafe = false>
auto read_op(std::span<const std::byte> code, Listener &listener) -> status {
  return detail::code_reader<Listener, unsafe>(code, listener).read_op();
}

template <typename Listener>
auto read_op_unsafe(std::span<const std::byte> code, Listener &listener) -> status {
  return read_op<Listener, true>(code, listener);
}

auto read_ops(std::span<const std::byte> code, code_listener &listener)
  -> std::expected<void, std::vector<error>>;

} // namespace lamavm::bc
