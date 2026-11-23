#include "mmap.hpp"

#include "core/assert.hpp"
#include "page.hpp"

#include <cstddef>
#include <cstdint>
#include <span>
#include <tuple>
#include <utility>

#include <sys/mman.h>

namespace lamavm::os {

void mprotect(std::span<std::byte> bytes, int prot) {
  if (bytes.empty()) {
    return;
  }

  const auto size = page_size();
  LAMAVM_EXPECT_B(reinterpret_cast<std::uintptr_t>(bytes.data()) % size, ==, 0U);
  LAMAVM_EXPECT_B(bytes.size() % size, ==, 0U);

  LAMAVM_EXPECT_B(::mprotect(bytes.data(), bytes.size(), prot), ==, 0);
}

mmap::mmap(std::size_t length, int prot) {
  LAMAVM_EXPECT_B(length, !=, 0U);
  LAMAVM_EXPECT_B(length % page_size(), ==, 0U);

  void* mapped = ::mmap(nullptr, length, prot, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  LAMAVM_EXPECT_B(mapped, !=, MAP_FAILED);

  bytes = {static_cast<std::byte*>(mapped), length};
}

mmap::mmap(mmap&& other) noexcept {
  swap(other);
}

mmap& mmap::operator=(mmap&& other) noexcept {
  mmap previous(std::move(other));
  swap(previous);
  return *this;
}

mmap::~mmap() {
  if (bytes.empty()) {
    return;
  }

  std::ignore = ::munmap(bytes.data(), bytes.size());
}

mmap::operator std::span<std::byte>() {
  return bytes;
}

mmap::operator std::span<const std::byte>() const {
  return bytes;
}

std::size_t mmap::pages() const {
  return bytes.size() / page_size();
}

std::span<std::byte> mmap::pages(std::size_t begin, std::size_t end) {
  LAMAVM_EXPECT_B(begin, <=, end);
  LAMAVM_EXPECT_B(end, <=, pages());

  const auto page = page_size();
  return bytes.subspan(begin * page, (end - begin) * page);
}

void mmap::swap(mmap& other) noexcept {
  std::swap(bytes, other.bytes);
}

} // namespace lamavm::os
