#include "handler.hpp"
#include "state.hpp"

#include "interpreter/runtime/box.hpp"
#include "interpreter/runtime/state.hpp"
#include "interpreter/std/std.g.hpp"
#include "interpreter/std/std.hpp"

#include "op/word.hpp"

#include "bc/units.hpp"

#include "core/assert.hpp"
#include "core/log.hpp"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <ranges>
#include <span>
#include <vector>

namespace lamavm::interpreter::cold {

namespace {

template <box (*f)(box)>
class unop {
public:
  static void operator()() {
    const auto x = stack_pop_arg();
    cold::stack_push(f(x));
  }
};

template <box (*f)(box, box)>
class binop {
public:
  static void operator()() {
    const box y = stack_pop_arg();
    const box x = stack_pop_arg();

    // Arithmetic overflow and division by zero
    // are intentionally left to the runtime.
    const box r = f(x, y);

    cold::stack_push(r);
  }
};

box& op_glo_target(std::int32_t& index) {
  std::size_t glo_base = up->glo_base;
  if (index < 0) {
    const bc::symbol& s = symbols.at(std::string_view{c_str(-index - 1)});
    LAMAVM_EXPECT_B(s.kind, ==, bc::symbol::kind::global);

    glo_base = units[s.uidx].glo_base;

    index = s.offset;
  }

  return stack[stack.size() - 1 - index - glo_base];
}

box& op_loc_target(std::int32_t index) {
  return fp[-frame::size - index];
}

box& op_arg_target(std::int32_t index) {
  const std::int64_t n_args = stack_n_args();
  return fp[n_args - index];
}

box& op_clo_target(std::int32_t index) {
  const std::int64_t n_args = stack_n_args();
  auto* cj = unbox<box*>(fp[1 + n_args]);
  return cj[1 + 1 + index];
}

} // namespace

void op_add() {
  binop<boxd::Ls__Infix_43>::operator()();
  LAMAVM_DISPATCH();
}

void op_sub() {
  binop<boxd::Ls__Infix_45>::operator()();
  LAMAVM_DISPATCH();
}

void op_mul() {
  binop<boxd::Ls__Infix_42>::operator()();
  LAMAVM_DISPATCH();
}

void op_div() {
  binop<boxd::Ls__Infix_47>::operator()();
  LAMAVM_DISPATCH();
}

void op_mod() {
  binop<boxd::Ls__Infix_37>::operator()();
  LAMAVM_DISPATCH();
}

void op_lt() {
  binop<boxd::Ls__Infix_60>::operator()();
  LAMAVM_DISPATCH();
}

void op_le() {
  binop<boxd::Ls__Infix_6061>::operator()();
  LAMAVM_DISPATCH();
}

void op_gt() {
  binop<boxd::Ls__Infix_62>::operator()();
  LAMAVM_DISPATCH();
}

void op_ge() {
  binop<boxd::Ls__Infix_6261>::operator()();
  LAMAVM_DISPATCH();
}

void op_eq() {
  binop<boxd::Ls__Infix_6161>::operator()();
  LAMAVM_DISPATCH();
}

void op_ne() {
  binop<boxd::Ls__Infix_3361>::operator()();
  LAMAVM_DISPATCH();
}

void op_and() {
  binop<boxd::Ls__Infix_3838>::operator()();
  LAMAVM_DISPATCH();
}

void op_or() {
  binop<boxd::Ls__Infix_3333>::operator()();
  LAMAVM_DISPATCH();
}

void op_const() {
  const auto [value, arg1] = (*++pc).as_i32;
  LAMAVM_EXPECT_B(arg1, ==, 0);

  cold::stack_push(box::checked(value));

  LAMAVM_DISPATCH();
}

void op_string() {
  const auto [string_offset, arg1] = (*++pc).as_i32;
  LAMAVM_EXPECT_B(arg1, ==, 0);

  const char* str1 = c_str(string_offset);
  const box str2 = gc_call([&] { return boxd::Bstring(box(static_cast<const void*>(&str1))); });
  cold::stack_push(str2);

  LAMAVM_DISPATCH();
}

void op_sexp() {
  const auto tag_offset = (*++pc).as_i32.arg0;
  const auto n_fields = (*++pc).as_i32.arg0;

  box* args = --sp;
  args[0] = boxd::LtagHash(box(c_str(tag_offset)));
  std::reverse(args, args + n_fields + 1);

  const box sexp = gc_call([&] { return boxd::Bsexp(box(args), box(n_fields + 1)); });

  sp += n_fields + 1;
  cold::stack_push(sexp);

  LAMAVM_DISPATCH();
}

void op_sta() {
  const box value = stack_pop_arg();
  const box index = stack_pop_arg();
  const box aggregate = stack_pop_arg();
  tracef(
    "sta {}[{}] := {}",
    unbox<void*>(aggregate),
    unbox<std::int64_t>(index),
    unbox<std::int64_t>(value));
  boxd::Bsta(aggregate, index, value);
  cold::stack_push(value);

  LAMAVM_DISPATCH();
}

void op_jmp() {
  const auto target = (*++pc).as_i32.arg0;
  pc = up->text.data() + target;

  LAMAVM_DISPATCH_JUMP();
}

void op_end() {
  const box x = stack_pop_arg();
  stack_pop_frame();
  cold::stack_push(x);

  LAMAVM_DISPATCH_JUMP();
}

void op_drop() {
  stack_pop_arg();

  LAMAVM_DISPATCH();
}

void op_dup() {
  const auto x = stack_pop_arg();
  cold::stack_push(x);
  cold::stack_push(x);

  LAMAVM_DISPATCH();
}

void op_swap() {
  const auto x = stack_pop_arg();
  const auto y = stack_pop_arg();
  cold::stack_push(x);
  cold::stack_push(y);

  LAMAVM_DISPATCH();
}

void op_elem() {
  const box index = stack_pop_arg();
  const box aggregate = stack_pop_arg();
  tracef("elem {}[{}]", unbox<void*>(aggregate), unbox<std::int64_t>(index));
  cold::stack_push(boxd::Belem(aggregate, index));

  LAMAVM_DISPATCH();
}

void op_ld_glo() {
  auto index = (*++pc).as_i32.arg0;
  auto& target = op_glo_target(index);
  tracef("ld_glo {} = {}", index, unbox<std::int64_t>(target));
  cold::stack_push(target);

  LAMAVM_DISPATCH();
}

void op_ld_loc() {
  const auto index = (*++pc).as_i32.arg0;
  auto loc = op_loc_target(index);
  tracef("ld_loc {} = {}", index, unbox<std::int64_t>(loc));
  cold::stack_push(loc);

  LAMAVM_DISPATCH();
}

void op_ld_arg() {
  const auto index = (*++pc).as_i32.arg0;
  const auto arg = op_arg_target(index);
  tracef("ld_arg {} = {}", index, unbox<std::int64_t>(arg));
  cold::stack_push(arg);

  LAMAVM_DISPATCH();
}

void op_ld_clo() {
  const auto index = (*++pc).as_i32.arg0;
  const auto arg = op_clo_target(index);
  tracef("ld_clo {} = {}", index, unbox<std::int64_t>(arg));
  cold::stack_push(arg);

  LAMAVM_DISPATCH();
}

void op_st_glo() {
  auto index = (*++pc).as_i32.arg0;
  auto& target = op_glo_target(index);

  const auto value = stack_pop_arg();
  cold::stack_push(value);

  tracef("sd_glo {} := {}", index, unbox<std::int64_t>(value));
  target = value;

  LAMAVM_DISPATCH();
}

void op_st_loc() {
  const auto index = (*++pc).as_i32.arg0;
  const auto value = stack_pop_arg();
  cold::stack_push(value);

  auto& loc = op_loc_target(index);
  tracef("st_loc {} := {}", index, unbox<std::int64_t>(value));
  loc = value;

  LAMAVM_DISPATCH();
}

void op_st_arg() {
  const auto index = (*++pc).as_i32.arg0;

  const auto value = stack_pop_arg();
  cold::stack_push(value);

  auto& arg = op_arg_target(index);
  tracef("st_arg {} := {}", index, unbox<std::int64_t>(value));
  arg = value;

  LAMAVM_DISPATCH();
}

void op_st_clo() {
  const auto index = (*++pc).as_i32.arg0;

  const auto value = stack_pop_arg();
  cold::stack_push(value);

  auto& arg = op_clo_target(index);
  tracef("st_clo {} := {}", index, unbox<std::int64_t>(value));
  arg = value;

  LAMAVM_DISPATCH();
}

void op_cjmp_z() {
  const auto target = (*++pc).as_i32.arg0;
  const auto value = stack_pop_arg();
  ++pc;

  if (unbox<std::int64_t>(value) == 0) {
    pc = up->text.data() + target;
  }

  LAMAVM_DISPATCH_JUMP();
}

void op_cjmp_nz() {
  const auto target = (*++pc).as_i32.arg0;
  const auto value = stack_pop_arg();
  ++pc;

  if (unbox<std::int64_t>(value) != 0) {
    pc = up->text.data() + target;
  }

  LAMAVM_DISPATCH_JUMP();
}

void op_begin() {
  const auto& args = (*++pc).as_i32;
  const auto n_args = args.arg0;
  const auto n_locals = args.arg1;
  LAMAVM_ASSERT_B(n_args, ==, stack_n_args());

  for (std::int32_t i = 0; i < n_locals; ++i) {
    cold::stack_push(box(std::int64_t{0}));
  }

  LAMAVM_DISPATCH();
}

void op_begin_closure() {
  const auto& args = (*++pc).as_i32;
  const auto n_args = args.arg0;
  const auto n_locals = args.arg1;
  [[maybe_unused]] const auto n_captures = (*++pc).as_i32.arg0;

  LAMAVM_ASSERT_B(n_args, ==, stack_n_args());

  for (std::int32_t i = 0; i < n_locals; ++i) {
    cold::stack_push(box(std::int64_t{0}));
  }

  LAMAVM_DISPATCH();
}

void op_closure() {
  auto target = (*++pc).as_i32.arg0;
  const auto n_captured = (*++pc).as_i32.arg1;

  const auto* target_up = up;
  if (target < 0) { // repeated with call
    const bc::symbol& s = symbols.at(std::string_view{c_str(-target - 1)});
    LAMAVM_EXPECT_B(s.kind, ==, bc::symbol::kind::function);

    target_up = units.data() + s.uidx;
    target = s.offset;
  }

  const std::span<const op::word_t> captured(pc + 1, n_captured);
  pc += n_captured;

  for (const op::word_t c : std::ranges::views::reverse(captured)) {
    const auto args = c.as_i32;
    const std::int32_t kind = args.arg0;
    std::int32_t index = args.arg1;

    box value;
    switch (kind) {
    case 0:
      value = op_glo_target(index);
      break;
    case 1:
      value = op_loc_target(index);
      break;
    case 2:
      value = op_arg_target(index);
      break;
    case 3:
      value = op_clo_target(index);
      break;
    default:
      fatalf("unexpected capture kind {:x}", kind);
    }

    cold::stack_push(value);
  }

  cold::stack_push(box(target_up));
  cold::stack_push(box(target));

  const box cj = gc_call([&] { return boxd::Bclosure(box(sp), box(1 + n_captured)); });

  sp += 1 + 1 + n_captured;

  cold::stack_push(cj);

  LAMAVM_DISPATCH();
}

void op_callc() {
  const auto args = (*++pc).as_i32;
  const auto n_args = args.arg0;

  auto* cj = unbox<box*>(*(sp + n_args));
  auto target = unbox<std::int32_t>(cj[0]);
  auto* target_up = unbox<uctx*>(cj[1]);

  stack_push_frame({
    .n_args = n_args,
    .fp = fp,
    .pc = pc + 1,
    .sp = sp + n_args + 1,
    .up = up,
  });

  up = target_up;
  pc = up->text.data() + target;
  LAMAVM_DISPATCH_JUMP();
}

void op_call() {
  auto target = (*++pc).as_i32.arg0;
  const auto n_args = (*++pc).as_i32.arg1;

  std::string_view name;
  if (target < 0) {
    name = c_str(-target - 1);

    // T O D O: collisions between symbols and std_symbols
    if (const auto it = std_symbols.find(name); it != std::end(std_symbols)) {
      const auto& [_, s] = *it;

      switch (s.tail) {
      case std_fn_symbol::tail_kind::none:
        LAMAVM_EXPECT_B(s.head, ==, n_args);
        break;
      case std_fn_symbol::tail_kind::vararg:
        LAMAVM_EXPECT_B(s.head, <=, n_args);
        break;
      }

      std::span<box> rargs{sp, sp + n_args};
      const box r = gc_call([&] { return s.fn(rargs); });
      sp += n_args;
      cold::stack_push(r);

      LAMAVM_DISPATCH();
    }
  }

  stack_push_frame({
    .n_args = n_args,
    .fp = fp,
    .pc = pc + 1,
    .sp = sp + n_args,
    .up = up,
  });

  if (!name.empty()) {
    const auto s = symbols.at(name);
    LAMAVM_EXPECT_B(s.kind, ==, bc::symbol::kind::function);

    up = units.data() + s.uidx;

    target = s.offset;
  }

  pc = up->text.data() + target;
  LAMAVM_DISPATCH_JUMP();
}

void op_tag() {
  const auto tag_offset = (*++pc).as_i32.arg0;
  const auto n_fields = (*++pc).as_i32.arg0;

  const box tag_hash = boxd::LtagHash(box(c_str(tag_offset)));
  const box value = stack_pop_arg();
  cold::stack_push(boxd::Btag(value, tag_hash, box(n_fields)));

  LAMAVM_DISPATCH();
}

void op_array() {
  const auto n_elements = (*++pc).as_i32.arg0;
  const box value = stack_pop_arg();
  tracef("array {}", n_elements);
  cold::stack_push(boxd::Barray_patt(value, box(n_elements)));

  LAMAVM_DISPATCH();
}

void op_fail() {
  const auto args = (*++pc).as_i32;
  const auto line = args.arg0;
  const auto col = args.arg1;
  const box value = stack_pop_arg();
  tracef("fail {}:{}:{}", up->name, line, col);
  boxd::Bmatch_failure(value, box(up->name), box(line), box(col));
}

void op_fail_keep() {
  const auto args = (*++pc).as_i32;
  const auto line = args.arg0;
  const auto col = args.arg1;
  tracef("fail_keep {}:{}:{}", up->name, line, col);
  boxd::Bmatch_failure(*sp, box(up->name), box(line), box(col));
}

void op_line() {
  ++pc;

  LAMAVM_DISPATCH();
}

void op_patt_str_cmp() {
  binop<boxd::Bstring_patt>::operator()();

  LAMAVM_DISPATCH();
}

void op_patt_string() {
  unop<boxd::Bstring_tag_patt>::operator()();

  LAMAVM_DISPATCH();
}

void op_patt_array() {
  unop<boxd::Barray_tag_patt>::operator()();

  LAMAVM_DISPATCH();
}

void op_patt_sexp() {
  unop<boxd::Bsexp_tag_patt>::operator()();

  LAMAVM_DISPATCH();
}

void op_patt_boxed() {
  unop<boxd::Bboxed_patt>::operator()();

  LAMAVM_DISPATCH();
}

void op_patt_unboxed() {
  unop<boxd::Bunboxed_patt>::operator()();

  LAMAVM_DISPATCH();
}

void op_patt_closure() {
  unop<boxd::Bclosure_tag_patt>::operator()();

  LAMAVM_DISPATCH();
}

void op_barray() {
  const auto n_elements = (*++pc).as_i32.arg0;
  tracef("barray {}", n_elements);

  box* args = sp;
  std::reverse(args, args + n_elements);

  const box array = gc_call([&] { return boxd::Barray(box(args), box(n_elements)); });

  sp += n_elements;
  cold::stack_push(array);

  LAMAVM_DISPATCH();
}

void op_eof() {
}

} // namespace lamavm::interpreter::cold
