#include "loop.hpp"

#include "runtime.hpp"

#include "bc/layout.hpp"
#include "bc/unit.hpp"
#include "core/assert.hpp"
#include "core/log.hpp"

#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <span>
#include <string_view>
#include <utility>
#include <vector>

// extern "C" {
// #include <contrib/runtime/gc.h>
// }

#include <atomic>
#include <cstddef>

namespace lamavm::interpreter {

namespace {

std::vector<char> buffer;

std::string_view data;
std::span<const std::uint8_t> text;
std::vector<std::int64_t> stack;

const std::uint8_t* ip = nullptr;
std::int64_t* sp = nullptr;
std::int64_t* fp = nullptr;

} // namespace

namespace {

struct frame {
  std::int64_t* fp = nullptr;
  const std::uint8_t* ip = nullptr;
  std::int64_t* sp = nullptr;
};

// const char* c_str(std::int32_t offset) {
//   return data.data() + offset;
// }

template <typename T>
T text_next() {
  T x;
  std::memcpy(&x, ip, sizeof(T));
  ip += sizeof(T);
  return x;
}

void stack_push(std::int64_t x) {
  --sp;
  *sp = x;
}

void stack_push(const void* x) {
  stack_push(reinterpret_cast<std::int64_t>(x));
}

std::int64_t stack_pop() {
  const std::int64_t x = *sp;
  sp++;
  return x;
}

void stack_push_frame(frame f) {
  std::int64_t* fp1 = sp;
  stack_push(f.fp);
  stack_push(f.ip);
  stack_push(f.sp);
  fp = fp1;
}

void stack_pop_frame() {
  sp = reinterpret_cast<std::int64_t*>(fp[-3]); // NOLINT(performance-no-int-to-ptr)
  ip = reinterpret_cast<std::uint8_t*>(fp[-2]); // NOLINT(performance-no-int-to-ptr)
  fp = reinterpret_cast<std::int64_t*>(fp[-1]); // NOLINT(performance-no-int-to-ptr)
}

} // namespace

namespace {

void op_add() {
  const auto y = stack_pop();
  const auto x = stack_pop();
  stack_push(Ls__Infix_43(x, y));
}

void op_sub() {
  const auto y = stack_pop();
  const auto x = stack_pop();
  stack_push(Ls__Infix_45(x, y));
}

void op_mul() {
  const auto y = stack_pop();
  const auto x = stack_pop();
  stack_push(Ls__Infix_42(x, y));
}

void op_div() {
  const auto y = stack_pop();
  const auto x = stack_pop();
  stack_push(Ls__Infix_47(x, y));
}

void op_mod() {
  const auto y = stack_pop();
  const auto x = stack_pop();
  stack_push(Ls__Infix_37(x, y));
}

void op_lt() {
  const auto y = stack_pop();
  const auto x = stack_pop();
  stack_push(Ls__Infix_60(x, y));
}

void op_le() {
  const auto y = stack_pop();
  const auto x = stack_pop();
  stack_push(Ls__Infix_6061(x, y));
}

void op_gt() {
  const auto y = stack_pop();
  const auto x = stack_pop();
  stack_push(Ls__Infix_62(x, y));
}

void op_ge() {
  const auto y = stack_pop();
  const auto x = stack_pop();
  stack_push(Ls__Infix_6261(x, y));
}

void op_eq() {
  const auto y = stack_pop();
  const auto x = stack_pop();
  stack_push(Ls__Infix_6161(x, y));
}

void op_ne() {
  const auto y = stack_pop();
  const auto x = stack_pop();
  stack_push(Ls__Infix_3361(x, y));
}

void op_and() {
  const auto y = stack_pop();
  const auto x = stack_pop();
  stack_push(Ls__Infix_3838(x, y));
}

void op_or() {
  const auto y = stack_pop();
  const auto x = stack_pop();
  stack_push(Ls__Infix_3333(x, y));
}

void op_const() {
  const auto value = text_next<std::int32_t>();
  stack_push(box(value));
}

void op_string() {
  fatalf("unexpected opcode {}", bc::opcode_t::op_string);
}

void op_sexp() {
  fatalf("unexpected opcode {}", bc::opcode_t::op_sexp);
}

void op_sta() {
  fatalf("unexpected opcode {}", bc::opcode_t::op_sta);
}

void op_jmp() {
  fatalf("unexpected opcode {}", bc::opcode_t::op_jmp);
}

void op_drop() {
  stack_pop();
}

void op_dup() {
  const auto x = stack_pop();
  stack_push(x);
  stack_push(x);
}

void op_swap() {
  const auto x = stack_pop();
  const auto y = stack_pop();
  stack_push(x);
  stack_push(y);
}

void op_elem() {
  fatalf("unexpected opcode {}", bc::opcode_t::op_elem);
}

void op_ld_glo() {
  fatalf("unexpected opcode {}", bc::opcode_t::op_ld_glo);
}

void op_ld_loc() {
  fatalf("unexpected opcode {}", bc::opcode_t::op_ld_loc);
}

void op_ld_arg() {
  fatalf("unexpected opcode {}", bc::opcode_t::op_ld_arg);
}

void op_ld_clo() {
  fatalf("unexpected opcode {}", bc::opcode_t::op_ld_clo);
}

void op_st_glo() {
  fatalf("unexpected opcode {}", bc::opcode_t::op_st_glo);
}

void op_st_loc() {
  fatalf("unexpected opcode {}", bc::opcode_t::op_st_loc);
}

void op_st_arg() {
  fatalf("unexpected opcode {}", bc::opcode_t::op_st_arg);
}

void op_st_clo() {
  fatalf("unexpected opcode {}", bc::opcode_t::op_st_clo);
}

void op_cjmp_z() {
  fatalf("unexpected opcode {}", bc::opcode_t::op_cjmp_z);
}

void op_cjmp_nz() {
  fatalf("unexpected opcode {}", bc::opcode_t::op_cjmp_nz);
}

void op_callc() {
  fatalf("unexpected opcode {}", bc::opcode_t::op_callc);
}

void op_call() {
  // T O D O: call push_stack_frame here
  fatalf("unexpected opcode {}", bc::opcode_t::op_call);
}

void op_begin() {
  const auto n_args = text_next<std::int32_t>();
  const auto n_locals = text_next<std::int32_t>();

  (void)n_args; // T O D O : why is it needed?

  for (std::int32_t i = 0; i < n_locals; ++i) {
    stack_push(box(0));
  }
}

void op_begin_closure() {
  fatalf("unexpected opcode {}", bc::opcode_t::op_begin_closure);
}

void op_closure() {
  fatalf("unexpected opcode {}", bc::opcode_t::op_closure);
}

void op_end() {
  const std::int64_t x = stack_pop();
  stack_pop_frame();
  stack_push(x);
}

void op_tag() {
  fatalf("unexpected opcode {}", bc::opcode_t::op_tag);
}

void op_array() {
  fatalf("unexpected opcode {}", bc::opcode_t::op_array);
}

void op_fail() {
  fatalf("unexpected opcode {}", bc::opcode_t::op_fail);
}

void op_fail_keep() {
  fatalf("unexpected opcode {}", bc::opcode_t::op_fail_keep);
}

void op_line() {
  fatalf("unexpected opcode {}", bc::opcode_t::op_line);
}

void op_patt_str_cmp() {
  fatalf("unexpected opcode {}", bc::opcode_t::op_patt_str_cmp);
}

void op_patt_string() {
  fatalf("unexpected opcode {}", bc::opcode_t::op_patt_string);
}

void op_patt_array() {
  fatalf("unexpected opcode {}", bc::opcode_t::op_patt_array);
}

void op_patt_sexp() {
  fatalf("unexpected opcode {}", bc::opcode_t::op_patt_sexp);
}

void op_patt_boxed() {
  fatalf("unexpected opcode {}", bc::opcode_t::op_patt_boxed);
}

void op_patt_unboxed() {
  fatalf("unexpected opcode {}", bc::opcode_t::op_patt_unboxed);
}

void op_patt_closure() {
  fatalf("unexpected opcode {}", bc::opcode_t::op_patt_closure);
}

void op_barray() {
  fatalf("unexpected opcode {}", bc::opcode_t::op_barray);
}

void op_eof() {
  std::exit(0); // NOLINT(concurrency-mt-unsafe)
}

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define LAMAVM_CASE(op)                                                                            \
  case bc::opcode_t::op:                                                                           \
    op();                                                                                          \
    return;

void step() {
  const auto offset = ip - text.data();
  const auto opcode = text_next<bc::opcode_t>();
  tracef("step {} ip={}", opcode, offset);

  switch (opcode) {
    LAMAVM_CASE(op_add)
    LAMAVM_CASE(op_sub)
    LAMAVM_CASE(op_mul)
    LAMAVM_CASE(op_div)
    LAMAVM_CASE(op_mod)
    LAMAVM_CASE(op_lt)
    LAMAVM_CASE(op_le)
    LAMAVM_CASE(op_gt)
    LAMAVM_CASE(op_ge)
    LAMAVM_CASE(op_eq)
    LAMAVM_CASE(op_ne)
    LAMAVM_CASE(op_and)
    LAMAVM_CASE(op_or)
    LAMAVM_CASE(op_const)
    LAMAVM_CASE(op_string)
    LAMAVM_CASE(op_sexp)
    LAMAVM_CASE(op_sta)
    LAMAVM_CASE(op_jmp)
    LAMAVM_CASE(op_end)
    LAMAVM_CASE(op_drop)
    LAMAVM_CASE(op_dup)
    LAMAVM_CASE(op_swap)
    LAMAVM_CASE(op_elem)
    LAMAVM_CASE(op_ld_glo)
    LAMAVM_CASE(op_ld_loc)
    LAMAVM_CASE(op_ld_arg)
    LAMAVM_CASE(op_ld_clo)
    LAMAVM_CASE(op_st_glo)
    LAMAVM_CASE(op_st_loc)
    LAMAVM_CASE(op_st_arg)
    LAMAVM_CASE(op_st_clo)
    LAMAVM_CASE(op_cjmp_z)
    LAMAVM_CASE(op_cjmp_nz)
    LAMAVM_CASE(op_begin)
    LAMAVM_CASE(op_begin_closure)
    LAMAVM_CASE(op_closure)
    LAMAVM_CASE(op_callc)
    LAMAVM_CASE(op_call)
    LAMAVM_CASE(op_tag)
    LAMAVM_CASE(op_array)
    LAMAVM_CASE(op_fail)
    LAMAVM_CASE(op_fail_keep)
    LAMAVM_CASE(op_line)
    LAMAVM_CASE(op_patt_str_cmp)
    LAMAVM_CASE(op_patt_string)
    LAMAVM_CASE(op_patt_array)
    LAMAVM_CASE(op_patt_sexp)
    LAMAVM_CASE(op_patt_boxed)
    LAMAVM_CASE(op_patt_unboxed)
    LAMAVM_CASE(op_patt_closure)
    LAMAVM_CASE(op_barray)
    LAMAVM_CASE(op_eof)
  }

  fatalf("unexpected opcode {}", opcode);
}

#undef LAMAVM_CASE

} // namespace

void run(bc::unit_t unit) {
  static std::atomic_bool is_loaded = false;
  LAMAVM_ASSERT(!is_loaded.exchange(true));

  buffer = std::move(unit.buffer);
  data = unit.string_table;
  text = unit.code;

  // T O D O : mmap guard, mmap stack, mmap guard
  constexpr size_t stack_size = size_t{1024} * 1024 / sizeof(std::size_t);
  stack = std::vector<int64_t>(stack_size);

  ip = text.data();
  sp = stack.data() + stack.size();
  fp = sp;

  LAMAVM_ASSERT_B(0U, <, text.size());
  LAMAVM_ASSERT_B(static_cast<bc::opcode_t>(text.back()), ==, bc::opcode_t::op_eof);
  stack_push_frame(frame{.fp = fp, .ip = text.data() + text.size() - 1, .sp = sp});

  for (;;) {
    step();
  }
}

} // namespace lamavm::interpreter
