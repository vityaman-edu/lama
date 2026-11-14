#include "units.hpp"

#include "limit.hpp"
#include "unit.hpp"

#include "core/assert.hpp"

#include <cstddef>
#include <filesystem>
#include <format>
#include <string>
#include <string_view>
#include <unordered_set>
#include <utility>

namespace lamavm::bc {

namespace {

struct loader {
  std::size_t size = 0;
  units u;
  std::unordered_set<std::string_view> started;
};

// NOLINTNEXTLINE(misc-no-recursion)
void visit(loader& l, const std::filesystem::path& path) {
  std::string name = path.stem().string();
  if (l.u.by_name.contains(name)) {
    return;
  }

  const bool is_new_start = l.started.insert(name).second;
  LAMAVM_EXPECT_C(is_new_start, std::format("cyclic import on: {}", name));

  auto u = unit::parse(path);

  const auto size = unit::size(u);
  LAMAVM_EXPECT_B(size, <=, max_packed_unit_size - l.size);
  l.size += size;

  for (const auto import : u.imports) {
    if (import == "Std") {
      continue;
    }

    auto subpath = path.parent_path() / import;
    subpath += ".bc";
    visit(l, subpath);
  }

  l.started.erase(name);

  const auto [it, is_new] = l.u.by_name.emplace(std::move(name), std::move(u));
  LAMAVM_EXPECT(is_new);

  l.u.initialization_order.push_back(it);
}

} // namespace

units units::from(const std::filesystem::path& path) {
  loader l;
  visit(l, path);

  return std::move(l.u);
}

} // namespace lamavm::bc
