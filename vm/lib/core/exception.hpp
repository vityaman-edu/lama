#pragma once

#include <expected>
#include <stdexcept>
#include <string>

namespace lamavm {

using error = std::string;
using status = std::expected<void, error>;

class exception : public std::runtime_error {
public:
  using std::runtime_error::runtime_error;
};

} // namespace lamavm
