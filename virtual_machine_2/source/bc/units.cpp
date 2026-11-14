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

  for (const auto i : u.imports) {
    if (i == "Std") {
      continue;
    }

    auto subpath = path.parent_path() / i;
    subpath += ".bc";
    visit(l, subpath);
  }

  const auto [it1, is_new] = l.u.by_name.emplace(std::move(name), std::move(u));
  LAMAVM_EXPECT(is_new);
  const auto& [name1, u1] = *it1;

  for (const auto& [name, s] : u1.public_symbols) {
    const auto [it2, is_new] = l.u.symbols.emplace(name, units::symbol{s, it1});
    const auto& [_, s2] = *it2;
    const auto& [name2, u2] = *s2.unit;
    LAMAVM_EXPECT_C(
      is_new,
      std::format("ambiguous public symbol '{}' from units '{}' and '{}'", name, name1, name2));
  }

  l.u.initialization_order.push_back(it1);
}

} // namespace

units units::from(const std::filesystem::path& path) {
  loader l;
  visit(l, path);

  return std::move(l.u);
}

} // namespace lamavm::bc
