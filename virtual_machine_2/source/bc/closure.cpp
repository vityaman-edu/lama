#include "closure.hpp"

#include "bc/layout.hpp"
#include "bc/limit.hpp"
#include "core/assert.hpp"

#include <algorithm>
#include <cstddef>
#include <string>
#include <string_view>
#include <unordered_set>
#include <utility>

namespace lamavm::bc {

namespace {

struct preclosure {
  std::vector<std::string_view> order;
  std::unordered_map<std::string, unit> unit_by_name;

  static preclosure open(const std::filesystem::path& path);
};

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

preclosure preclosure::open(const std::filesystem::path& path) {
  preclosure1 cj;
  cj.visit(path);
  return std::move(static_cast<preclosure&>(cj));
}

std::size_t closure_size(const unit& main, const preclosure& cj) {
  std::size_t size = sizeof(header_t);
  size += sizeof(import_t); // only Std
  size += main.public_symbols().size() * sizeof(public_symbol_t);
  for (const auto& [_, u] : cj.unit_by_name) {
    size += u.string_table().size();
    size += u.code_section().size();
    size -= 1; // EOF
  }
  size += sizeof(op_begin_t);
  size += sizeof(op_call_t) * cj.order.size();
  size += sizeof(op_drop_t) * (cj.order.size() - 1);
  size += sizeof(op_end_t);
  size += 1; // EOF

  return size;
}

unit closure(preclosure cj) {
  LAMAVM_ASSERT_B(0, <, cj.order.size());
  const auto& main = cj.unit_by_name.at(std::string(cj.order.back()));

  std::size_t size = closure_size(main, cj);
  // Relocations:
  // - public_symbols (merge in-memory for easy resolution)
  //     name_offset  DO (string table moved)
  //     code_offset  DO (code moved, globals shifted)
  // - JMP/CJMP       DO (code moved)
  // - CALL           DO (code moved, string_table moved)
  // - STRING         DO (string table moved)
  // - LD/ST GLO      DO (globals shifted, string_table moved)
  // other not implemented yet
}

} // namespace

unit closure(const std::filesystem::path& path) {
  preclosure cj = preclosure::open(path);
  return closure(std::move(cj));
}

} // namespace lamavm::bc
