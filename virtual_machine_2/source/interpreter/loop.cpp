#include "loop.hpp"

#include "runtime.hpp"

#include "op/handler.hpp"
#include "op/word.hpp"

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
std::span<const op::word_t> text;
std::vector<std::int64_t> stack;

const op::word_t* ip = nullptr;
std::int64_t* sp = nullptr;
std::int64_t* fp = nullptr;

} // namespace

namespace {

struct frame {
  std::int64_t n_args = 0;
  std::int64_t* fp = nullptr;
  const op::word_t* ip = nullptr;
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
  sp = reinterpret_cast<std::int64_t*>(fp[-3]);     // NOLINT(performance-no-int-to-ptr)
  ip = reinterpret_cast<const op::word_t*>(fp[-2]); // NOLINT(performance-no-int-to-ptr)
  fp = reinterpret_cast<std::int64_t*>(fp[-1]);     // NOLINT(performance-no-int-to-ptr)
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
  const auto value = (*ip++).as_i32.arg0;
  stack_push(box(value));
}

void op_string() {
  const auto string_offset = (*ip++).as_i32.arg0;
  const char* str1 = c_str(string_offset);
  const void* str2 = gc_call([&] { return Bstring(static_cast<const void*>(&str1)); });
  stack_push(str2);
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

void op_ld_glo() {
  const auto index = (*ip++).as_i32.arg0;
  LAMAVM_ASSERT_B(0, <=, index);

  auto value = stack[stack.size() - 1 - index];
  tracef("ld_glo {} = {}", index, unbox(value));
  stack_push(value);
}

void op_ld_loc() {
  const auto index = (*ip++).as_i32.arg0;
  const auto loc = fp[-4 - index];
  tracef("ld_loc {} = {}", index, unbox(loc));
  stack_push(loc);
}

void op_ld_arg() {
  const auto index = (*ip++).as_i32.arg0;
  const auto n_args = stack_n_args();
  const auto arg = fp[n_args - index];
  tracef("ld_arg {} = {}", index, unbox(arg));
  stack_push(arg);
}

void op_st_glo() {
  const auto index = (*ip++).as_i32.arg0;
  LAMAVM_ASSERT_B(0, <=, index);

  const auto value = stack_pop();
  stack_push(value);

  auto& target = stack[stack.size() - 1 - index];
  tracef("sd_glo {} := {}", index, unbox(value));
  target = value;
}

void op_st_loc() {
  const auto index = (*ip++).as_i32.arg0;
  const auto value = stack_pop();
  stack_push(value);

  auto& loc = fp[-4 - index];
  tracef("st_loc {} := {}", index, unbox(value));
  loc = value;
}

void op_st_arg() {
  const auto index = (*ip++).as_i32.arg0;
  const auto n_args = stack_n_args();

  const auto value = stack_pop();
  stack_push(value);

  auto& arg = fp[n_args - index];
  tracef("st_arg {} := {}", index, unbox(value));
  arg = value;
}

void op_jmp() {
  const auto target = (*ip++).as_i32.arg0;
  ip = text.data() + target;
}

void op_cjmp_z() {
  const auto target = (*ip++).as_i32.arg0;
  const auto value = stack_pop();

  if (unbox(value) == 0) {
    ip = text.data() + target;
  }
}

void op_cjmp_nz() {
  const auto target = (*ip++).as_i32.arg0;
  const auto value = stack_pop();

  if (unbox(value) != 0) {
    ip = text.data() + target;
  }
}

void op_call() {
  const auto args = (*ip++).as_i32;
  const auto target = args.arg0;
  const auto n_args = args.arg1;
  LAMAVM_ASSERT_B(0, <=, target);
  stack_push_frame({.n_args = n_args, .fp = fp, .ip = ip, .sp = sp + n_args});
  ip = text.data() + target;
}

void op_begin() {
  const auto args = (*ip++).as_i32;
  [[maybe_unused]] const auto n_args = args.arg0;
  const auto n_locals = args.arg1;
  LAMAVM_ASSERT_B(n_args, ==, stack_n_args());

  for (std::int32_t i = 0; i < n_locals; ++i) {
    stack_push(static_cast<std::int64_t>(0));
  }
}

void op_end() {
  const std::int64_t x = stack_pop();
  stack_pop_frame();
  stack_push(x);
}

void op_line() {
  ++ip;
}

void op_eof() {
  std::exit(0); // NOLINT(concurrency-mt-unsafe)
}

void step() {
  const auto offset = ip - text.data();
  const auto handler_id = (*ip++).as_handler_id;
  tracef("step {} ip={}", handler_id, offset);

  switch (handler_id) {
  case static_cast<op::handler_id_t>(bc::opcode_t::op_add):
    op_add();
    return;
  case static_cast<op::handler_id_t>(bc::opcode_t::op_sub):
    op_sub();
    return;
  case static_cast<op::handler_id_t>(bc::opcode_t::op_mul):
    op_mul();
    return;
  case static_cast<op::handler_id_t>(bc::opcode_t::op_div):
    op_div();
    return;
  case static_cast<op::handler_id_t>(bc::opcode_t::op_mod):
    op_mod();
    return;
  case static_cast<op::handler_id_t>(bc::opcode_t::op_lt):
    op_lt();
    return;
  case static_cast<op::handler_id_t>(bc::opcode_t::op_le):
    op_le();
    return;
  case static_cast<op::handler_id_t>(bc::opcode_t::op_gt):
    op_gt();
    return;
  case static_cast<op::handler_id_t>(bc::opcode_t::op_ge):
    op_ge();
    return;
  case static_cast<op::handler_id_t>(bc::opcode_t::op_eq):
    op_eq();
    return;
  case static_cast<op::handler_id_t>(bc::opcode_t::op_ne):
    op_ne();
    return;
  case static_cast<op::handler_id_t>(bc::opcode_t::op_and):
    op_and();
    return;
  case static_cast<op::handler_id_t>(bc::opcode_t::op_or):
    op_or();
    return;
  case static_cast<op::handler_id_t>(bc::opcode_t::op_const):
    op_const();
    return;
  case static_cast<op::handler_id_t>(bc::opcode_t::op_string):
    op_string();
    return;
  case static_cast<op::handler_id_t>(bc::opcode_t::op_drop):
    op_drop();
    return;
  case static_cast<op::handler_id_t>(bc::opcode_t::op_dup):
    op_dup();
    return;
  case static_cast<op::handler_id_t>(bc::opcode_t::op_swap):
    op_swap();
    return;
  case static_cast<op::handler_id_t>(bc::opcode_t::op_ld_glo):
    op_ld_glo();
    return;
  case static_cast<op::handler_id_t>(bc::opcode_t::op_ld_loc):
    op_ld_loc();
    return;
  case static_cast<op::handler_id_t>(bc::opcode_t::op_ld_arg):
    op_ld_arg();
    return;
  case static_cast<op::handler_id_t>(bc::opcode_t::op_st_glo):
    op_st_glo();
    return;
  case static_cast<op::handler_id_t>(bc::opcode_t::op_st_loc):
    op_st_loc();
    return;
  case static_cast<op::handler_id_t>(bc::opcode_t::op_st_arg):
    op_st_arg();
    return;
  case static_cast<op::handler_id_t>(bc::opcode_t::op_jmp):
    op_jmp();
    return;
  case static_cast<op::handler_id_t>(bc::opcode_t::op_cjmp_z):
    op_cjmp_z();
    return;
  case static_cast<op::handler_id_t>(bc::opcode_t::op_cjmp_nz):
    op_cjmp_nz();
    return;
  case static_cast<op::handler_id_t>(bc::opcode_t::op_call):
    op_call();
    return;
  case static_cast<op::handler_id_t>(bc::opcode_t::op_begin):
    op_begin();
    return;
  case static_cast<op::handler_id_t>(bc::opcode_t::op_end):
    op_end();
    return;
  case static_cast<op::handler_id_t>(bc::opcode_t::op_line):
    op_line();
    return;
  case static_cast<op::handler_id_t>(bc::opcode_t::op_eof):
    op_eof();
    return;
  default:
    fatalf("unexpected handler {}", handler_id);
  }
}

} // namespace

void run(const bc::unit& unit) {
  [[maybe_unused]] static std::atomic_bool is_loaded = false;
  LAMAVM_EXPECT(!is_loaded.exchange(true));

  data = unit.string_table;
  text = unit.text;

  constexpr size_t stack_size = size_t{1024} * 1024 / sizeof(std::size_t);
  stack = std::vector<int64_t>(stack_size);

  ip = text.data();
  sp = stack.data() + stack.size();
  fp = sp;

  gc_init();
  gc_set_stack_bottom(sp);

  LAMAVM_ASSERT_B(0U, <, text.size());
  LAMAVM_ASSERT_B(
    text.back().as_handler_id,
    ==,
    static_cast<op::handler_id_t>(bc::opcode_t::op_eof));

  for (std::size_t i = 0; i < unit.globals_count; ++i) {
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
