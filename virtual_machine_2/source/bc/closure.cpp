#include "closure.hpp"

#include "bc/limit.hpp"
#include "core/assert.hpp"

#include <algorithm>
#include <string>
#include <string_view>
#include <unordered_set>
#include <utility>

namespace lamavm::bc {

namespace {

class preclosure1 : public preclosure {
public:
  // NOLINTNEXTLINE(misc-no-recursion)
  void visit(const std::filesystem::path& path) {
    std::string name = path.stem().string();
    if (unit_by_name.contains(name)) {
      return;
    }

    {
      const auto [_, is_newbie] = started.insert(name);
      LAMAVM_EXPECT_C(is_newbie, std::format("cyclic import on: {}", name));
    }

    auto u = unit::parse(path);
    size += u.size();
    LAMAVM_EXPECT_B(size, <=, max_unit_size);

    for (const auto& import : u.imports()) {
      std::string_view name(u.string(import.name_offset));

      auto subpath = path.parent_path() / name;
      subpath += ".bc";

      visit(subpath);
    }

    const auto [it, is_newbie] = unit_by_name.emplace(std::move(name), std::move(u));
    LAMAVM_EXPECT(is_newbie);

    const std::string& newname = it->first;
    order.emplace_back(newname);
  }

private:
  std::size_t size = 0;
  std::unordered_set<std::string_view> started;
};

} // namespace

preclosure preclosure::open(const std::filesystem::path& path) {
  preclosure1 cj;
  cj.visit(path);
  return std::move(static_cast<preclosure&>(cj));
}

unit closure(preclosure units) {

}

} // namespace lamavm::bc
