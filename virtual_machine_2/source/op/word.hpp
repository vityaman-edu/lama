#pragma once

#include "handler.hpp"

#include "core/c.hpp"

#include <concepts>
#include <cstddef>
#include <cstdint>
#include <span>

namespace lamavm::interpreter {
class box;
} // namespace lamavm::interpreter

namespace lamavm::op {

using arg_idx_t = std::int32_t; // argument
using loc_idx_t = std::int32_t; // local variable
using glo_idx_t = std::int32_t; // global variable

struct call_t {
  std::uint32_t unit_index;
  std::int32_t count;
};

struct capture_t {
  static constexpr std::int32_t global_base = -1;
  static constexpr std::int32_t frame_base = 0;

  std::int32_t base;
  std::int32_t offset;
};

// LamaVM text (code) is just an array of opwords.
// One operation (instruction) consists of opwords.
// The first operation opword is always a handler.
// The following operation opwords are arguments.
//
// LamaVM text is an extensible representation
// and is used in both plantime and runtime.
// E.g. at plantime, handler is a `handler_id_t`.
// E.g. at runtime, handler is a `handler_fn_t`.
//
// There are following representations currently supported:
// - Naive. The handler is an id, arguments are offsets.
// - Threaded. The handler is fn, arguments are pointers.
union word_t {
  handler_id_t as_handler_id; // for a ptr comparison
  handler_fn_t as_handler_fn; // like the `opcode_t`, but at the runtime
  word_t* as_target;          // e.g. JMP, CALL
  struct {
    std::int32_t arg0;
    std::int32_t arg1;
  } as_i32; // e.g. SEXP
  capture_t as_capture;
  const char* as_str;
  std::uint64_t as_box_bits;
  interpreter::box* as_slot;
  interpreter::box (*as_std_fn)(std::span<interpreter::box>);
  call_t as_call;
};

static_assert(ctype<word_t>);
static_assert(sizeof(word_t) == sizeof(std::uintptr_t));

template <typename Layout = word_t>
  requires(std::same_as<Layout, word_t> || requires { &Layout::handler; })
[[nodiscard]] constexpr std::ptrdiff_t word_count() {
  constexpr auto word_size = sizeof(word_t);
  static_assert(sizeof(Layout) % word_size == 0);
  return static_cast<std::ptrdiff_t>(sizeof(Layout) / word_size);
}

} // namespace lamavm::op
