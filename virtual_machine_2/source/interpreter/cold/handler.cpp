#include "handler.hpp"

#include "interpreter/runtime/box.hpp"
#include "interpreter/runtime/state.hpp"
#include "interpreter/runtime/std.hpp"

#include "op/word.hpp"

#include "bc/units.hpp"

#include "core/assert.hpp"
#include "core/log.hpp"

#include <cstddef>
#include <cstdint>
#include <cstdlib>

namespace lamavm::interpreter::cold {

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

box& op_glo_target(std::int32_t& index) {
  index = (*++pc).as_i32.arg0;

  std::size_t glo_base = up->glo_base;
  if (index < 0) {
    const bc::symbol& s = symbols.at(std::string_view{c_str(-index - 1)});
    LAMAVM_EXPECT_B(s.kind, ==, bc::symbol::kind::global);

    glo_base = units[s.uidx].glo_base;

    index = s.offset;
  }

  return stack[stack.size() - 1 - index - glo_base];
}

} // namespace

void op_add() {
  binop<Ls__Infix_43>::operator()();

  LAMAVM_DISPATCH();
}

void op_sub() {
  binop<Ls__Infix_45>::operator()();

  LAMAVM_DISPATCH();
}

void op_mul() {
  binop<Ls__Infix_42>::operator()();

  LAMAVM_DISPATCH();
}

void op_div() {
  binop<Ls__Infix_47>::operator()();

  LAMAVM_DISPATCH();
}

void op_mod() {
  binop<Ls__Infix_37>::operator()();

  LAMAVM_DISPATCH();
}

void op_lt() {
  binop<Ls__Infix_60>::operator()();

  LAMAVM_DISPATCH();
}

void op_le() {
  binop<Ls__Infix_6061>::operator()();

  LAMAVM_DISPATCH();
}

void op_gt() {
  binop<Ls__Infix_62>::operator()();

  LAMAVM_DISPATCH();
}

void op_ge() {
  binop<Ls__Infix_6261>::operator()();

  LAMAVM_DISPATCH();
}

void op_eq() {
  binop<Ls__Infix_6161>::operator()();

  LAMAVM_DISPATCH();
}

void op_ne() {
  binop<Ls__Infix_3361>::operator()();

  LAMAVM_DISPATCH();
}

void op_and() {
  binop<Ls__Infix_3838>::operator()();

  LAMAVM_DISPATCH();
}

void op_or() {
  binop<Ls__Infix_3333>::operator()();

  LAMAVM_DISPATCH();
}

void op_const() {
  const auto value = (*++pc).as_i32.arg0;
  stack_push(box(value));

  LAMAVM_DISPATCH();
}

void op_string() {
  const auto string_offset = (*++pc).as_i32.arg0;
  const char* str1 = c_str(string_offset);
  const box str2 = gc_call([&] { return Bstring(box(static_cast<const void*>(&str1))); });
  stack_push(str2);

  LAMAVM_DISPATCH();
}

void op_drop() {
  stack_pop();

  LAMAVM_DISPATCH();
}

void op_dup() {
  const auto x = stack_pop();
  stack_push(x);
  stack_push(x);

  LAMAVM_DISPATCH();
}

void op_swap() {
  const auto x = stack_pop();
  const auto y = stack_pop();
  stack_push(x);
  stack_push(y);

  LAMAVM_DISPATCH();
}

void op_ld_glo() {
  std::int32_t index = 0;
  auto& target = op_glo_target(index);
  tracef("ld_glo {} = {}", index, unbox<std::int64_t>(target));
  stack_push(target);

  LAMAVM_DISPATCH();
}

void op_ld_loc() {
  const auto index = (*++pc).as_i32.arg0;
  const auto loc = fp[-frame::size - index];
  tracef("ld_loc {} = {}", index, unbox<std::int64_t>(loc));
  stack_push(loc);

  LAMAVM_DISPATCH();
}

void op_ld_arg() {
  const auto index = (*++pc).as_i32.arg0;
  const auto n_args = stack_n_args();
  const auto arg = fp[n_args - index];
  tracef("ld_arg {} = {}", index, unbox<std::int64_t>(arg));
  stack_push(arg);

  LAMAVM_DISPATCH();
}

void op_st_glo() {
  std::int32_t index = 0;
  auto& target = op_glo_target(index);

  const auto value = stack_pop();
  stack_push(value);

  tracef("sd_glo {} := {}", index, unbox<std::int64_t>(value));
  target = value;

  LAMAVM_DISPATCH();
}

void op_st_loc() {
  const auto index = (*++pc).as_i32.arg0;
  const auto value = stack_pop();
  stack_push(value);

  auto& loc = fp[-frame::size - index];
  tracef("st_loc {} := {}", index, unbox<std::int64_t>(value));
  loc = value;

  LAMAVM_DISPATCH();
}

void op_st_arg() {
  const auto index = (*++pc).as_i32.arg0;
  const auto n_args = stack_n_args();

  const auto value = stack_pop();
  stack_push(value);

  auto& arg = fp[n_args - index];
  tracef("st_arg {} := {}", index, unbox<std::int64_t>(value));
  arg = value;

  LAMAVM_DISPATCH();
}

void op_jmp() {
  const auto target = (*++pc).as_i32.arg0;
  pc = up->text.data() + target;

  LAMAVM_DISPATCH_JUMP();
}

void op_cjmp_z() {
  const auto target = (*++pc).as_i32.arg0;
  const auto value = stack_pop();
  ++pc;

  if (unbox<std::int64_t>(value) == 0) {
    pc = up->text.data() + target;
  }

  LAMAVM_DISPATCH_JUMP();
}

void op_cjmp_nz() {
  const auto target = (*++pc).as_i32.arg0;
  const auto value = stack_pop();
  ++pc;

  if (unbox<std::int64_t>(value) != 0) {
    pc = up->text.data() + target;
  }

  LAMAVM_DISPATCH_JUMP();
}

void op_call() {
  const auto args = (*++pc).as_i32;
  std::int32_t target = args.arg0;
  const std::int32_t n_args = args.arg1;

  stack_push_frame({
    .n_args = n_args,
    .fp = fp,
    .pc = pc + 1,
    .sp = sp + n_args,
    .up = up,
  });

  if (target < 0) {
    const bc::symbol& s = symbols.at(std::string_view{c_str(-target - 1)});
    LAMAVM_EXPECT_B(s.kind, ==, bc::symbol::kind::function);

    up = units.data() + s.uidx;

    target = s.offset;
  }

  pc = up->text.data() + target;

  LAMAVM_DISPATCH_JUMP();
}

void op_begin() {
  const auto args = (*++pc).as_i32;
  const auto n_args = args.arg0;
  const auto n_locals = args.arg1;
  LAMAVM_ASSERT_B(n_args, ==, stack_n_args());

  for (std::int32_t i = 0; i < n_locals; ++i) {
    stack_push(box{});
  }

  LAMAVM_DISPATCH();
}

void op_end() {
  const box x = stack_pop();
  stack_pop_frame();
  stack_push(x);

  LAMAVM_DISPATCH_JUMP();
}

void op_line() {
  ++pc;

  LAMAVM_DISPATCH();
}

void op_eof() {
}

} // namespace lamavm::interpreter::cold
