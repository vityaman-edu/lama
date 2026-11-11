#include "file_listener_seq.hpp"

#include "core/exception.hpp"
#include "file_listener.hpp"
#include "layout.hpp"

#include <cstddef>
#include <expected>
#include <memory>
#include <span>
#include <string_view>
#include <utility>
#include <vector>

namespace lamavm::bc {

file_listener_seq::file_listener_seq(
  std::unique_ptr<file_listener> first,
  std::unique_ptr<file_listener> second)
  : listeners{std::move(first), std::move(second)} {
}

auto file_listener_seq::operator()(const header &op) -> status {
  for (const auto &listener : listeners) {
    if (auto result = (*listener)(op); !result) {
      return result;
    }
  }

  return {};
}

auto file_listener_seq::operator()(const import &op) -> status {
  for (const auto &listener : listeners) {
    if (auto result = (*listener)(op); !result) {
      return result;
    }
  }

  return {};
}

auto file_listener_seq::operator()(const public_symbol &op) -> status {
  for (const auto &listener : listeners) {
    if (auto result = (*listener)(op); !result) {
      return result;
    }
  }

  return {};
}

auto file_listener_seq::operator()(std::string_view strings) -> status {
  for (const auto &listener : listeners) {
    if (auto result = (*listener)(strings); !result) {
      return result;
    }
  }

  return {};
}

auto file_listener_seq::operator()(std::span<const std::byte> code)
  -> std::expected<void, std::vector<error>> {
  std::vector<error> errors;
  bool is_valid = true;
  for (const auto &listener : listeners) {
    if (auto result = (*listener)(code); !result) {
      is_valid = false;
      for (auto &message : result.error()) {
        errors.push_back(std::move(message));
      }
    }
  }

  if (!is_valid) {
    return std::unexpected(std::move(errors));
  }

  return {};
}

} // namespace lamavm::bc
