#include "loop.hpp"

#include "runtime.hpp"

#include "bc/decode.hpp"
#include "bc/layout.hpp"
#include "bc/unit.hpp"
#include "core/assert.hpp"
#include "core/log.hpp"

#include <cstdint>
#include <cstdlib>
#include <span>
#include <vector>

#include <atomic>
#include <cstddef>

namespace lamavm::interpreter {

namespace {

std::span<const char> data;
std::span<const std::byte> text;
std::vector<std::int64_t> stack;

const std::byte* ip = nullptr;
std::int64_t* sp = nullptr;
std::int64_t* fp = nullptr;

} // namespace

namespace {

struct frame {
  std::int64_t n_args = 0;
  std::int64_t* fp = nullptr;
  const std::byte* ip = nullptr;
  std::int64_t* sp = nullptr;
};

const char* c_str(std::int32_t offset) {
  return data.data() + offset;
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
  stack_push(box(f.n_args));
  std::int64_t* fp1 = sp;
  stack_push(f.fp);
  stack_push(f.ip);
  stack_push(f.sp);
  fp = fp1;
}

void stack_pop_frame() {
  sp = reinterpret_cast<std::int64_t*>(fp[-3]);    // NOLINT(performance-no-int-to-ptr)
  ip = reinterpret_cast<const std::byte*>(fp[-2]); // NOLINT(performance-no-int-to-ptr)
  fp = reinterpret_cast<std::int64_t*>(fp[-1]);    // NOLINT(performance-no-int-to-ptr)
}

std::int64_t stack_n_args() {
  return unbox(fp[0]);
}

auto gc_call(auto f) {
  gc_set_stack_top(sp - 1);
  return f();
}

} // namespace

namespace {

void execute([[maybe_unused]] const bc::op_add_t& op) {
  const auto y = stack_pop();
  const auto x = stack_pop();
  stack_push(Ls__Infix_43(x, y));
}

void execute([[maybe_unused]] const bc::op_sub_t& op) {
  const auto y = stack_pop();
  const auto x = stack_pop();
  stack_push(Ls__Infix_45(x, y));
}

void execute([[maybe_unused]] const bc::op_mul_t& op) {
  const auto y = stack_pop();
  const auto x = stack_pop();
  stack_push(Ls__Infix_42(x, y));
}

void execute([[maybe_unused]] const bc::op_div_t& op) {
  const auto y = stack_pop();
  const auto x = stack_pop();
  stack_push(Ls__Infix_47(x, y));
}

void execute([[maybe_unused]] const bc::op_mod_t& op) {
  const auto y = stack_pop();
  const auto x = stack_pop();
  stack_push(Ls__Infix_37(x, y));
}

void execute([[maybe_unused]] const bc::op_lt_t& op) {
  const auto y = stack_pop();
  const auto x = stack_pop();
  stack_push(Ls__Infix_60(x, y));
}

void execute([[maybe_unused]] const bc::op_le_t& op) {
  const auto y = stack_pop();
  const auto x = stack_pop();
  stack_push(Ls__Infix_6061(x, y));
}

void execute([[maybe_unused]] const bc::op_gt_t& op) {
  const auto y = stack_pop();
  const auto x = stack_pop();
  stack_push(Ls__Infix_62(x, y));
}

void execute([[maybe_unused]] const bc::op_ge_t& op) {
  const auto y = stack_pop();
  const auto x = stack_pop();
  stack_push(Ls__Infix_6261(x, y));
}

void execute([[maybe_unused]] const bc::op_eq_t& op) {
  const auto y = stack_pop();
  const auto x = stack_pop();
  stack_push(Ls__Infix_6161(x, y));
}

void execute([[maybe_unused]] const bc::op_ne_t& op) {
  const auto y = stack_pop();
  const auto x = stack_pop();
  stack_push(Ls__Infix_3361(x, y));
}

void execute([[maybe_unused]] const bc::op_and_t& op) {
  const auto y = stack_pop();
  const auto x = stack_pop();
  stack_push(Ls__Infix_3838(x, y));
}

void execute([[maybe_unused]] const bc::op_or_t& op) {
  const auto y = stack_pop();
  const auto x = stack_pop();
  stack_push(Ls__Infix_3333(x, y));
}

void execute(const bc::op_const_t& op) {
  stack_push(box(op.value));
}

void execute(const bc::op_string_t& op) {
  const char* str1 = c_str(op.string_offset);
  const void* str2 = gc_call([&] { return Bstring(static_cast<const void*>(&str1)); });
  stack_push(str2);
}

void execute([[maybe_unused]] const bc::op_drop_t& op) {
  stack_pop();
}

void execute([[maybe_unused]] const bc::op_dup_t& op) {
  const auto x = stack_pop();
  stack_push(x);
  stack_push(x);
}

void execute([[maybe_unused]] const bc::op_swap_t& op) {
  const auto x = stack_pop();
  const auto y = stack_pop();
  stack_push(x);
  stack_push(y);
}

void execute(const bc::op_ld_glo_t& op) {
  LAMAVM_ASSERT_B(0, <=, op.index);

  auto value = stack[stack.size() - 1 - op.index];
  tracef("ld_glo {} = {}", op.index, unbox(value));
  stack_push(value);
}

void execute(const bc::op_ld_loc_t& op) {
  const auto loc = fp[-4 - op.index];
  tracef("ld_loc {} = {}", op.index, unbox(loc));
  stack_push(loc);
}

void execute(const bc::op_ld_arg_t& op) {
  const auto n_args = stack_n_args();
  const auto arg = fp[n_args - op.index];
  tracef("ld_arg {} = {}", op.index, unbox(arg));
  stack_push(arg);
}

void execute(const bc::op_st_glo_t& op) {
  LAMAVM_ASSERT_B(0, <=, op.index);

  const auto value = stack_pop();
  stack_push(value);

  auto& target = stack[stack.size() - 1 - op.index];
  tracef("sd_glo {} := {}", op.index, unbox(value));
  target = value;
}

void execute(const bc::op_st_loc_t& op) {
  const auto value = stack_pop();
  stack_push(value);

  auto& loc = fp[-4 - op.index];
  tracef("st_loc {} := {}", op.index, unbox(value));
  loc = value;
}

void execute(const bc::op_st_arg_t& op) {
  const auto n_args = stack_n_args();

  const auto value = stack_pop();
  stack_push(value);

  auto& arg = fp[n_args - op.index];
  tracef("st_arg {} := {}", op.index, unbox(value));
  arg = value;
}

void execute(const bc::op_jmp_t& op) {
  ip = text.data() + op.target;
}

void execute(const bc::op_cjmp_z_t& op) {
  const auto value = stack_pop();
  if (unbox(value) == 0) {
    ip = text.data() + op.target;
  }
}

void execute(const bc::op_cjmp_nz_t& op) {
  const auto value = stack_pop();
  if (unbox(value) != 0) {
    ip = text.data() + op.target;
  }
}

void execute(const bc::op_call_t& op) {
  [[maybe_unused]] const auto target = op.target;
  LAMAVM_ASSERT_B(0, <=, target);
  stack_push_frame({.n_args = op.n_args, .fp = fp, .ip = ip, .sp = sp + op.n_args});
  ip = text.data() + op.target;
}

void execute(const bc::op_begin_t& op) {
  [[maybe_unused]] const auto n_args = op.n_args;
  LAMAVM_ASSERT_B(n_args, ==, stack_n_args());

  for (std::int32_t i = 0; i < op.n_locals; ++i) {
    stack_push(static_cast<std::int64_t>(0));
  }
}

void execute([[maybe_unused]] const bc::op_end_t& op) {
  const std::int64_t x = stack_pop();
  stack_pop_frame();
  stack_push(x);
}

void execute([[maybe_unused]] const bc::op_line_t& op) {
}

void execute([[maybe_unused]] const bc::op_eof_t& op) {
  std::exit(0); // NOLINT(concurrency-mt-unsafe)
}

void step() {
  const auto offset = ip - text.data();
  auto visitor = [offset](const auto& op) {
    tracef("step {} ip={}", op.opcode, offset);
    execute(op);
  };
  bc::decode_fast(ip, visitor);
}

} // namespace

void run(const bc::unit& unit) {
  [[maybe_unused]] static std::atomic_bool is_loaded = false;
  LAMAVM_EXPECT(!is_loaded.exchange(true));

  data = unit.string_table();
  text = std::as_bytes(unit.code_section());

  constexpr size_t stack_size = size_t{1024} * 1024 / sizeof(std::size_t);
  stack = std::vector<int64_t>(stack_size);

  ip = text.data();
  sp = stack.data() + stack.size();
  fp = sp;

  gc_init();
  gc_set_stack_bottom(sp);

  LAMAVM_ASSERT_B(0U, <, text.size());
  LAMAVM_ASSERT_B(static_cast<bc::opcode_t>(text.back()), ==, bc::opcode_t::op_eof);

  for (std::int32_t i = 0; i < unit.header().globals_count; ++i) {
    stack_push(box(0));
  }

  stack_push(box(static_cast<std::int64_t>(0)));
  stack_push(box(static_cast<std::int64_t>(0)));
  stack_push_frame(frame{.n_args = 2, .fp = fp, .ip = text.data() + text.size() - 1, .sp = sp});

  for (;;) {
    step();
  }
}

} // namespace lamavm::interpreter
