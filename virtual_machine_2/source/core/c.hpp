#pragma once

#include <type_traits>

namespace lamavm {

template <typename T>
concept ctype = (std::is_class_v<T> || std::is_union_v<T>) &&
                std::is_standard_layout_v<T> &&
                std::is_aggregate_v<T> &&
                std::is_trivially_default_constructible_v<T> &&
                std::is_trivially_copyable_v<T> &&
                std::is_trivially_copy_assignable_v<T> &&
                std::is_trivially_destructible_v<T>;

} // namespace lamavm
