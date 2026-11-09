#pragma once

#include <string>
#include <string_view>

namespace lamavm::support {

[[nodiscard]] auto quote(std::string_view value) -> std::string;

}
