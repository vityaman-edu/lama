#include "string.hpp"

#include <format>
#include <string>
#include <string_view>

namespace lamavm::support {
namespace {

auto quote(unsigned char byte) -> std::string {
  switch (byte) {
  case '"':
    return "\\\"";
  case '\\':
    return "\\\\";
  case '\n':
    return "\\n";
  case '\r':
    return "\\r";
  case '\t':
    return "\\t";
  case '\b':
    return "\\b";
  default:
    if (byte >= ' ' && byte < '\x7f') {
      return {static_cast<char>(byte)};
    }

    return std::format("\\{:03d}", byte);
  }
}

} // namespace

auto quote(std::string_view value) -> std::string {
  std::string result = "\"";
  for (const unsigned char byte : value) {
    result += quote(byte);
  }

  result += '"';
  return result;
}

} // namespace lamavm::support
