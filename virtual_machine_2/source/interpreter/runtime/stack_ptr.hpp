#pragma once

#include <bit>
#include <cstddef>
#include <utility>

namespace lamavm::interpreter {

template <typename T>
class stack_ptr final {
public:
  explicit constexpr stack_ptr(std::size_t* gc_stack_top) noexcept
    : gc_stack_top(gc_stack_top) {
  }

  stack_ptr(const stack_ptr&) = delete;
  stack_ptr& operator=(const stack_ptr&) = delete;

  stack_ptr& operator=(T* value) noexcept {
    set(value);
    return *this;
  }

  explicit operator T*() const noexcept {
    return get();
  }

  decltype(auto) operator*(this auto&& self) noexcept {
    return std::forward_like<decltype(self)&>(*self.get());
  }

  T* operator->() const noexcept {
    return get();
  }

  decltype(auto) operator[](this auto&& self, std::size_t i) noexcept {
    return std::forward_like<decltype(self)&>(self.get()[i]);
  }

  [[nodiscard]] T* operator+(std::ptrdiff_t offset) const noexcept {
    return get() + offset;
  }

  [[nodiscard]] T* operator-(std::ptrdiff_t offset) const noexcept {
    return get() - offset;
  }

  stack_ptr& operator+=(std::ptrdiff_t offset) noexcept {
    return *this = *this + offset;
  }

  stack_ptr& operator-=(std::ptrdiff_t offset) noexcept {
    return *this = *this - offset;
  }

  stack_ptr& operator++() noexcept {
    return *this += 1;
  }

  stack_ptr& operator--() noexcept {
    return *this -= 1;
  }

  T* operator++(int) noexcept {
    auto* previous = get();
    ++*this;
    return previous;
  }

  T* operator--(int) noexcept {
    auto* previous = get();
    --*this;
    return previous;
  }

private:
  [[nodiscard]] T* get() const noexcept {
    const auto top = *gc_stack_top;
    return top == 0 ? nullptr : std::bit_cast<T*>(top + sizeof(T));
  }

  void set(T* value) noexcept {
    *gc_stack_top = value == nullptr ? 0 : std::bit_cast<std::size_t>(value) - sizeof(T);
  }

  std::size_t* gc_stack_top;
};

} // namespace lamavm::interpreter
