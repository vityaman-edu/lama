#include "loop.hpp"

#include "interpreter/runtime/box.hpp"
#include "interpreter/runtime/gc.hpp"
#include "interpreter/runtime/std.hpp"

#include "op/handler.hpp"
#include "op/word.hpp"

#include "bc/layout.hpp"
#include "bc/unit.hpp"

#include "core/assert.hpp"
#include "core/log.hpp"

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <limits>
#include <span>
#include <type_traits>
#include <vector>

namespace lamavm::interpreter {

namespace {

std::span<const char> data;
std::span<const op::word_t> text;
std::vector<box> stack;

const op::word_t* pc = nullptr;
box* sp = nullptr;
box* fp = nullptr;

} // namespace

namespace {

struct frame {
  std::int64_t n_args = 0;
  box* fp = nullptr;
  const op::word_t* pc = nullptr;
  box* sp = nullptr;
};

const char* c_str(std::int32_t offset) {
  return data.data() + offset;
}

void stack_push(box x) {
  --sp;
  *sp = x;
}

box stack_pop() {
  const box x = *sp;
  sp++;
  return x;
}

void stack_push_frame(frame f) {
  stack_push(box(f.n_args));
  box* fp1 = sp;
  stack_push(box(f.fp));
  stack_push(box(f.pc));
  stack_push(box(f.sp));
  fp = fp1;
}

void stack_pop_frame() {
  sp = unbox<box*>(fp[-3]);
  pc = unbox<const op::word_t*>(fp[-2]);
  fp = unbox<box*>(fp[-1]);
}

std::int64_t stack_n_args() {
  return unbox<std::int64_t>(fp[0]);
}

auto gc_call(auto f) {
  gc::set_top(sp - 1);
  return f();
}

} // namespace

namespace {

template <auto f>
class binop {
public:
  static void operator()() {
    const auto y = stack_pop();
    const auto x = stack_pop();
    stack_push(f(x, y));
  }
};

void op_const() {
  const auto value = (*pc++).as_i32.arg0;
  stack_push(box(value));
}

void op_string() {
  const auto string_offset = (*pc++).as_i32.arg0;
  const char* str1 = c_str(string_offset);
  const box str2 = gc_call([&] { return Bstring(box(static_cast<const void*>(&str1))); });
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
  const auto index = (*pc++).as_i32.arg0;
  LAMAVM_ASSERT_B(0, <=, index);

  auto value = stack[stack.size() - 1 - index];
  tracef("ld_glo {} = {}", index, unbox<std::int64_t>(value));
  stack_push(value);
}

void op_ld_loc() {
  const auto index = (*pc++).as_i32.arg0;
  const auto loc = fp[-4 - index];
  tracef("ld_loc {} = {}", index, unbox<std::int64_t>(loc));
  stack_push(loc);
}

void op_ld_arg() {
  const auto index = (*pc++).as_i32.arg0;
  const auto n_args = stack_n_args();
  const auto arg = fp[n_args - index];
  tracef("ld_arg {} = {}", index, unbox<std::int64_t>(arg));
  stack_push(arg);
}

void op_st_glo() {
  const auto index = (*pc++).as_i32.arg0;
  LAMAVM_ASSERT_B(0, <=, index);

  const auto value = stack_pop();
  stack_push(value);

  auto& target = stack[stack.size() - 1 - index];
  tracef("sd_glo {} := {}", index, unbox<std::int64_t>(value));
  target = value;
}

void op_st_loc() {
  const auto index = (*pc++).as_i32.arg0;
  const auto value = stack_pop();
  stack_push(value);

  auto& loc = fp[-4 - index];
  tracef("st_loc {} := {}", index, unbox<std::int64_t>(value));
  loc = value;
}

void op_st_arg() {
  const auto index = (*pc++).as_i32.arg0;
  const auto n_args = stack_n_args();

  const auto value = stack_pop();
  stack_push(value);

  auto& arg = fp[n_args - index];
  tracef("st_arg {} := {}", index, unbox<std::int64_t>(value));
  arg = value;
}

void op_jmp() {
  const auto target = (*pc++).as_i32.arg0;
  pc = text.data() + target;
}

void op_cjmp_z() {
  const auto target = (*pc++).as_i32.arg0;
  const auto value = stack_pop();

  if (unbox<std::int64_t>(value) == 0) {
    pc = text.data() + target;
  }
}

void op_cjmp_nz() {
  const auto target = (*pc++).as_i32.arg0;
  const auto value = stack_pop();

  if (unbox<std::int64_t>(value) != 0) {
    pc = text.data() + target;
  }
}

void op_call() {
  const auto args = (*pc++).as_i32;
  const auto target = args.arg0;
  const auto n_args = args.arg1;
  LAMAVM_ASSERT_B(0, <=, target);
  stack_push_frame({.n_args = n_args, .fp = fp, .pc = pc, .sp = sp + n_args});
  pc = text.data() + target;
}

void op_begin() {
  const auto args = (*pc++).as_i32;
  const auto n_args = args.arg0;
  const auto n_locals = args.arg1;
  LAMAVM_ASSERT_B(n_args, ==, stack_n_args());

  for (std::int32_t i = 0; i < n_locals; ++i) {
    stack_push(box{});
  }
}

void op_end() {
  const box x = stack_pop();
  stack_pop_frame();
  stack_push(x);
}

void op_line() {
  ++pc;
}

void op_eof() {
  std::exit(0); // NOLINT(concurrency-mt-unsafe)
}

void step() {
  const auto offset = pc - text.data();
  const auto handler_id = (*pc++).as_handler_id;
  tracef("step {} pc={}", handler_id, offset);

  const auto max = std::numeric_limits<std::underlying_type_t<bc::opcode_t>>::max();
  if (max < handler_id) {
    fatalf("unexpected handler overflow {:x}", handler_id);
  }

  switch (static_cast<bc::opcode_t>(handler_id)) {
  case bc::opcode_t::op_add:
    binop<Ls__Infix_43>::operator()();
    return;
  case bc::opcode_t::op_sub:
    binop<Ls__Infix_45>::operator()();
    return;
  case bc::opcode_t::op_mul:
    binop<Ls__Infix_42>::operator()();
    return;
  case bc::opcode_t::op_div:
    binop<Ls__Infix_47>::operator()();
    return;
  case bc::opcode_t::op_mod:
    binop<Ls__Infix_37>::operator()();
    return;
  case bc::opcode_t::op_lt:
    binop<Ls__Infix_60>::operator()();
    return;
  case bc::opcode_t::op_le:
    binop<Ls__Infix_6061>::operator()();
    return;
  case bc::opcode_t::op_gt:
    binop<Ls__Infix_62>::operator()();
    return;
  case bc::opcode_t::op_ge:
    binop<Ls__Infix_6261>::operator()();
    return;
  case bc::opcode_t::op_eq:
    binop<Ls__Infix_6161>::operator()();
    return;
  case bc::opcode_t::op_ne:
    binop<Ls__Infix_3361>::operator()();
    return;
  case bc::opcode_t::op_and:
    binop<Ls__Infix_3838>::operator()();
    return;
  case bc::opcode_t::op_or:
    binop<Ls__Infix_3333>::operator()();
    return;
  case bc::opcode_t::op_const:
    op_const();
    return;
  case bc::opcode_t::op_string:
    op_string();
    return;
  case bc::opcode_t::op_drop:
    op_drop();
    return;
  case bc::opcode_t::op_dup:
    op_dup();
    return;
  case bc::opcode_t::op_swap:
    op_swap();
    return;
  case bc::opcode_t::op_ld_glo:
    op_ld_glo();
    return;
  case bc::opcode_t::op_ld_loc:
    op_ld_loc();
    return;
  case bc::opcode_t::op_ld_arg:
    op_ld_arg();
    return;
  case bc::opcode_t::op_st_glo:
    op_st_glo();
    return;
  case bc::opcode_t::op_st_loc:
    op_st_loc();
    return;
  case bc::opcode_t::op_st_arg:
    op_st_arg();
    return;
  case bc::opcode_t::op_jmp:
    op_jmp();
    return;
  case bc::opcode_t::op_cjmp_z:
    op_cjmp_z();
    return;
  case bc::opcode_t::op_cjmp_nz:
    op_cjmp_nz();
    return;
  case bc::opcode_t::op_call:
    op_call();
    return;
  case bc::opcode_t::op_begin:
    op_begin();
    return;
  case bc::opcode_t::op_end:
    op_end();
    return;
  case bc::opcode_t::op_line:
    op_line();
    return;
  case bc::opcode_t::op_eof:
    op_eof();
    return;
  case bc::opcode_t::op_sexp:
  case bc::opcode_t::op_sta:
  case bc::opcode_t::op_elem:
  case bc::opcode_t::op_ld_clo:
  case bc::opcode_t::op_st_clo:
  case bc::opcode_t::op_begin_closure:
  case bc::opcode_t::op_closure:
  case bc::opcode_t::op_callc:
  case bc::opcode_t::op_tag:
  case bc::opcode_t::op_array:
  case bc::opcode_t::op_fail:
  case bc::opcode_t::op_fail_keep:
  case bc::opcode_t::op_patt_str_cmp:
  case bc::opcode_t::op_patt_string:
  case bc::opcode_t::op_patt_array:
  case bc::opcode_t::op_patt_sexp:
  case bc::opcode_t::op_patt_boxed:
  case bc::opcode_t::op_patt_unboxed:
  case bc::opcode_t::op_patt_closure:
  case bc::opcode_t::op_barray:
    break;
  }

  fatalf("unexpected handler {:x}", handler_id);
}

} // namespace

void run(const bc::units& units) {
  static std::atomic_bool is_loaded = false;
  LAMAVM_EXPECT(!is_loaded.exchange(true));

  data = unit.string_table;
  text = unit.text;

  constexpr size_t stack_size = size_t{1024} * 1024 / sizeof(box);
  stack = std::vector<box>(stack_size);

  pc = text.data();
  sp = stack.data() + stack.size();
  fp = sp;

  gc::init();
  gc::set_bot(sp);

  constexpr auto eof_id = static_cast<op::handler_id_t>(bc::opcode_t::op_eof);
  LAMAVM_ASSERT_B(0U, <, text.size());
  LAMAVM_ASSERT_B(text.back().as_handler_id, ==, eof_id);

  for (std::size_t i = 0; i < unit.globals_count; ++i) {
    stack_push(box(std::int64_t{0}));
  }

  stack_push(box(std::int64_t{0}));
  stack_push(box(std::int64_t{0}));
  stack_push_frame(frame{.n_args = 2, .fp = fp, .pc = text.data() + text.size() - 1, .sp = sp});

  for (;;) {
    step();
  }
}

} // namespace lamavm::interpreter
