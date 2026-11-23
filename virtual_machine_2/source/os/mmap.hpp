#pragma once

#include <cstddef>
#include <span>

namespace lamavm::os {

void mprotect(std::span<std::byte> bytes, int prot);

class mmap final {
public:
  mmap(std::size_t length, int prot);
  mmap(mmap&& other) noexcept;
  mmap(const mmap&) = delete;
  mmap& operator=(const mmap&) = delete;
  mmap& operator=(mmap&& other) noexcept;
  ~mmap();

  explicit operator std::span<std::byte>();
  explicit operator std::span<const std::byte>() const;

  [[nodiscard]] std::size_t pages() const;
  [[nodiscard]] std::span<std::byte> pages(std::size_t begin, std::size_t end);

private:
  void swap(mmap& other) noexcept;

  std::span<std::byte> bytes;
};

} // namespace lamavm::os
