#include "string.hpp"

#include <format>
#include <string>
#include <string_view>

namespace lamavm::support {

auto quote(std::string_view value) -> std::string {
  std::string result = "\"";
  for (const unsigned char byte : value) {
    switch (byte) {
    case '"':
      result += "\\\"";
      break;
    case '\\':
      result += "\\\\";
      break;
    case '\n':
      result += "\\n";
      break;
    case '\r':
      result += "\\r";
      break;
    case '\t':
      result += "\\t";
      break;
    case '\b':
      result += "\\b";
      break;
    default:
      if (byte >= ' ' && byte < '\x7f') {
        result += static_cast<char>(byte);
      } else {
        result += std::format("\\{:03d}", byte);
      }
    }
  }

  result += '"';
  return result;
}

} // namespace lamavm::support
