#include "file_listener_seq.hpp"

#include "file_listener.hpp"
#include "layout.hpp"

#include <cstddef>
#include <expected>
#include <memory>
#include <span>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace lamavm::bc {

file_listener_seq::file_listener_seq(
  std::unique_ptr<file_listener> first,
  std::unique_ptr<file_listener> second)
  : listeners{std::move(first), std::move(second)} {
}

void file_listener_seq::operator()(const header &op) {
  for (const auto &listener : listeners) {
    (*listener)(op);
  }
}

void file_listener_seq::operator()(const import &op) {
  for (const auto &listener : listeners) {
    (*listener)(op);
  }
}

void file_listener_seq::operator()(const public_symbol &op) {
  for (const auto &listener : listeners) {
    (*listener)(op);
  }
}

void file_listener_seq::operator()(std::string_view strings) {
  for (const auto &listener : listeners) {
    (*listener)(strings);
  }
}

auto file_listener_seq::operator()(std::span<const std::byte> code)
  -> std::expected<void, std::vector<std::string>> {
  std::vector<std::string> errors;
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
