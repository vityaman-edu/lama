#include "handler.hpp"
#include "state.hpp"

#include "interpreter/runtime/box.hpp"
#include "interpreter/runtime/state.hpp"
#include "interpreter/std/std.g.hpp"
#include "interpreter/std/std.hpp"

#include "op/word.hpp"

#include "bc/layout.hpp"
#include "bc/units.hpp"

#include "core/assert.hpp"
#include "core/log.hpp"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <format>
#include <iterator>
#include <ranges>
#include <span>
#include <string_view>
#include <vector>

namespace lamavm::interpreter::cold {

namespace {

void jmp(std::int32_t target) {
  LAMAVM_EXPECT_BB(0, <=, target, <, std::ssize(up->text));

  pc = up->text.data() + target;
}

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

const bc::symbol& find_symbol(std::string_view name) {
  const auto it = symbols.find(name);
  LAMAVM_EXPECT_C(it != symbols.end(), std::format("unknown symbol '{}'", name));

  return it->second;
}

struct glo_index {
  std::size_t glo_base;
  std::int32_t index;
};

glo_index op_glo_target_index(std::int32_t index) {
  const auto* owner = up;

  if (index < 0) {
    const auto& s = find_symbol(c_str(-(index + 1)));
    LAMAVM_EXPECT_B(s.kind, ==, bc::symbol::kind::global);

    owner = &units[s.uidx];
    index = s.offset;
  }

  LAMAVM_EXPECT_B(0, <=, index);

  const auto* next = owner + 1;
  const auto limit = next == units.data() + units.size() ? globals_count : next->glo_base;
  LAMAVM_EXPECT_B(static_cast<std::size_t>(index), <, limit - owner->glo_base);

  return {.glo_base = owner->glo_base, .index = index};
}

box& op_glo_target(glo_index target_index) {
  const auto [glo_base, index] = target_index;
  return stack[stack.size() - 1 - index - glo_base];
}

std::int64_t op_loc_target_index(std::int32_t index) {
  LAMAVM_EXPECT_BB(0, <=, index, <, stack_n_locals());

  return -frame::size - index;
}

box& op_loc_target(std::int64_t index) {
  return fp[index];
}

std::int64_t op_arg_target_index(std::int32_t index) {
  const auto n_args = std::ssize(stack_frame_args());
  LAMAVM_EXPECT_BB(0, <=, index, <, n_args);

  return n_args - index;
}

box& op_arg_target(std::int64_t index) {
  return fp[index];
}

std::span<box> closure_payload(box closure) {
  LAMAVM_EXPECT_C(unbox<void*>(closure) != nullptr, "expected a closure on the stack, got null");
  LAMAVM_EXPECT_C(
    unbox<std::int64_t>(boxd::Bclosure_tag_patt(closure)) == 1,
    "expected a closure on the stack");

  const auto size = unbox<std::int64_t>(boxd::Llength(closure));
  LAMAVM_EXPECT_B(2, <=, size);

  return {unbox<box*>(closure), static_cast<std::size_t>(size)};
}

std::size_t op_clo_target_index(std::int32_t index) {
  LAMAVM_EXPECT_B(0, <=, index);

  return 2 + static_cast<std::size_t>(index);
}

box& op_clo_target(std::size_t index) {
  const auto closure = closure_payload(stack_frame_closure());
  LAMAVM_EXPECT_B(index, <, closure.size());

  return closure[index];
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
  const auto [tag_offset, arg10] = (*++pc).as_i32;
  LAMAVM_EXPECT_B(arg10, ==, 0);

  const auto [n_fields, arg11] = (*++pc).as_i32;
  LAMAVM_EXPECT_B(arg11, ==, 0);
  LAMAVM_EXPECT_B(0, <=, n_fields);

  cold::stack_push() = boxd::LtagHash(box(c_str(tag_offset)));
  const auto args = stack_args(n_fields + 1);
  std::ranges::reverse(args);

  const box sexp = gc_call([&] { return boxd::Bsexp(box(args.data()), box(n_fields + 1)); });

  cold::stack_pop(n_fields + 1);
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
  const auto [target, arg1] = (*++pc).as_i32;
  LAMAVM_EXPECT_B(arg1, ==, 0);

  jmp(target);

  LAMAVM_DISPATCH_JUMP();
}

void op_end() {
  const box x = stack_pop_arg();
  cold::stack_pop_frame();
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
  const auto [operand, arg1] = (*++pc).as_i32;
  LAMAVM_EXPECT_B(arg1, ==, 0);

  const auto target_index = op_glo_target_index(operand);
  auto& target = op_glo_target(target_index);

  tracef("ld_glo {} = {}", target_index.index, unbox<std::int64_t>(target));
  cold::stack_push(target);

  LAMAVM_DISPATCH();
}

void op_ld_loc() {
  const auto [index, arg1] = (*++pc).as_i32;
  LAMAVM_EXPECT_B(arg1, ==, 0);

  auto loc = op_loc_target(op_loc_target_index(index));
  tracef("ld_loc {} = {}", index, unbox<std::int64_t>(loc));
  cold::stack_push(loc);

  LAMAVM_DISPATCH();
}

void op_ld_arg() {
  const auto [index, arg1] = (*++pc).as_i32;
  LAMAVM_EXPECT_B(arg1, ==, 0);

  const auto arg = op_arg_target(op_arg_target_index(index));
  tracef("ld_arg {} = {}", index, unbox<std::int64_t>(arg));
  cold::stack_push(arg);

  LAMAVM_DISPATCH();
}

void op_ld_clo() {
  const auto [index, arg1] = (*++pc).as_i32;
  LAMAVM_EXPECT_B(arg1, ==, 0);

  const auto arg = op_clo_target(op_clo_target_index(index));
  tracef("ld_clo {} = {}", index, unbox<std::int64_t>(arg));
  cold::stack_push(arg);

  LAMAVM_DISPATCH();
}

void op_st_glo() {
  const auto [operand, arg1] = (*++pc).as_i32;
  LAMAVM_EXPECT_B(arg1, ==, 0);

  const auto target_index = op_glo_target_index(operand);
  auto& target = op_glo_target(target_index);

  const auto value = stack_pop_arg();
  cold::stack_push(value);

  tracef("sd_glo {} := {}", target_index.index, unbox<std::int64_t>(value));
  target = value;

  LAMAVM_DISPATCH();
}

void op_st_loc() {
  const auto [index, arg1] = (*++pc).as_i32;
  LAMAVM_EXPECT_B(arg1, ==, 0);

  const auto value = stack_pop_arg();
  cold::stack_push(value);

  auto& loc = op_loc_target(op_loc_target_index(index));
  tracef("st_loc {} := {}", index, unbox<std::int64_t>(value));
  loc = value;

  LAMAVM_DISPATCH();
}

void op_st_arg() {
  const auto [index, arg1] = (*++pc).as_i32;
  LAMAVM_EXPECT_B(arg1, ==, 0);

  const auto value = stack_pop_arg();
  cold::stack_push(value);

  auto& arg = op_arg_target(op_arg_target_index(index));
  tracef("st_arg {} := {}", index, unbox<std::int64_t>(value));
  arg = value;

  LAMAVM_DISPATCH();
}

void op_st_clo() {
  const auto [index, arg1] = (*++pc).as_i32;
  LAMAVM_EXPECT_B(arg1, ==, 0);

  const auto value = stack_pop_arg();
  cold::stack_push(value);

  auto& arg = op_clo_target(op_clo_target_index(index));
  tracef("st_clo {} := {}", index, unbox<std::int64_t>(value));
  arg = value;

  LAMAVM_DISPATCH();
}

void op_cjmp_z() {
  const auto [target, arg1] = (*++pc).as_i32;
  LAMAVM_EXPECT_B(arg1, ==, 0);

  const auto value = stack_pop_arg();
  ++pc;

  if (unbox<std::int64_t>(value) == 0) {
    jmp(target);
  }

  LAMAVM_DISPATCH_JUMP();
}

void op_cjmp_nz() {
  const auto [target, arg1] = (*++pc).as_i32;
  LAMAVM_EXPECT_B(arg1, ==, 0);

  const auto value = stack_pop_arg();
  ++pc;

  if (unbox<std::int64_t>(value) != 0) {
    jmp(target);
  }

  LAMAVM_DISPATCH_JUMP();
}

void op_begin() {
  const auto [n_args, n_locals] = (*++pc).as_i32;
  stack_init_locals(n_args, n_locals);

  LAMAVM_DISPATCH();
}

void op_begin_closure() {
  const auto [n_args, n_locals] = (*++pc).as_i32;

  const auto [n_captures, arg1] = (*++pc).as_i32;
  LAMAVM_EXPECT_B(arg1, ==, 0);

  const auto closure = closure_payload(stack_frame_closure());
  LAMAVM_EXPECT_B(n_captures, ==, std::ssize(closure) - 2);

  stack_init_locals(n_args, n_locals);

  LAMAVM_DISPATCH();
}

void op_closure() {
  auto [target, arg1] = (*++pc).as_i32;
  LAMAVM_EXPECT_B(arg1, ==, 0);

  const auto [arg0, n_captured] = (*++pc).as_i32;
  LAMAVM_EXPECT_B(arg0, ==, 0);

  const auto* target_up = up;
  if (target < 0) {
    const auto& s = find_symbol(c_str(-(target + 1)));
    LAMAVM_EXPECT_B(s.kind, ==, bc::symbol::kind::function);

    target_up = units.data() + s.uidx;
    target = s.offset;
  }

  const std::span<const op::word_t> captured(pc + 1, n_captured);
  pc += n_captured;

  for (const op::word_t c : std::ranges::views::reverse(captured)) {
    const auto [kind, index] = c.as_i32;

    box value;
    switch (static_cast<bc::capture_kind>(kind)) {
    case bc::capture_kind::global:
      value = op_glo_target(op_glo_target_index(index));
      break;
    case bc::capture_kind::local:
      value = op_loc_target(op_loc_target_index(index));
      break;
    case bc::capture_kind::argument:
      value = op_arg_target(op_arg_target_index(index));
      break;
    case bc::capture_kind::closure:
      value = op_clo_target(op_clo_target_index(index));
      break;
    default:
      fatalf("unexpected capture kind {:x}", kind);
    }

    cold::stack_push(value);
  }

  cold::stack_push(box(target_up));
  cold::stack_push(box(target));

  const box cj = gc_call([&] { return boxd::Bclosure(box(sp), box(1 + n_captured)); });

  cold::stack_pop(std::int64_t{2} + n_captured);

  cold::stack_push(cj);

  LAMAVM_DISPATCH();
}

void op_callc() {
  const auto [n_args, arg1] = (*++pc).as_i32;
  LAMAVM_EXPECT_B(arg1, ==, 0);

  LAMAVM_EXPECT_B(0, <=, n_args);
  const auto args = stack_args(std::int64_t{n_args} + 1);
  const auto cj = closure_payload(args.back());
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
  jmp(target);
  LAMAVM_DISPATCH_JUMP();
}

void op_call() {
  auto [target, arg1] = (*++pc).as_i32;
  LAMAVM_EXPECT_B(arg1, ==, 0);

  const auto [arg0, n_args] = (*++pc).as_i32;
  LAMAVM_EXPECT_B(arg0, ==, 0);

  const auto args = stack_args(n_args);

  std::string_view name;
  if (target < 0) {
    name = c_str(-(target + 1));

    if (const auto it = std_symbols.find(name); it != std::end(std_symbols)) {
      LAMAVM_EXPECT_C(
        !symbols.contains(name),
        std::format("ambiguous symbol '{}' from bytecode and Std", name));

      const auto& [_, s] = *it;

      switch (s.tail) {
      case std_fn_symbol::tail_kind::none:
        LAMAVM_EXPECT_B(s.head, ==, n_args);
        break;
      case std_fn_symbol::tail_kind::vararg:
        LAMAVM_EXPECT_B(s.head, <=, n_args);
        break;
      }

      const box r = gc_call([&] { return s.fn(args); });
      cold::stack_pop(n_args);
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

  if (target < 0) {
    const auto& s = find_symbol(name);
    LAMAVM_EXPECT_B(s.kind, ==, bc::symbol::kind::function);

    up = units.data() + s.uidx;

    target = s.offset;
  }

  jmp(target);
  LAMAVM_DISPATCH_JUMP();
}

void op_tag() {
  const auto [tag_offset, arg10] = (*++pc).as_i32;
  LAMAVM_EXPECT_B(arg10, ==, 0);

  const auto [n_fields, arg11] = (*++pc).as_i32;
  LAMAVM_EXPECT_B(arg11, ==, 0);

  const box tag_hash = boxd::LtagHash(box(c_str(tag_offset)));
  const box value = stack_pop_arg();
  cold::stack_push(boxd::Btag(value, tag_hash, box(n_fields)));

  LAMAVM_DISPATCH();
}

void op_array() {
  const auto [n_elements, arg1] = (*++pc).as_i32;
  LAMAVM_EXPECT_B(arg1, ==, 0);

  const box value = stack_pop_arg();
  tracef("array {}", n_elements);
  cold::stack_push(boxd::Barray_patt(value, box(n_elements)));

  LAMAVM_DISPATCH();
}

void op_fail() {
  const auto [line, col] = (*++pc).as_i32;
  const box value = stack_pop_arg();
  tracef("fail {}:{}:{}", up->name, line, col);
  boxd::Bmatch_failure(value, box(up->name), box(line), box(col));
}

void op_fail_keep() {
  const auto [line, col] = (*++pc).as_i32;
  tracef("fail_keep {}:{}:{}", up->name, line, col);
  boxd::Bmatch_failure(stack_args(1).front(), box(up->name), box(line), box(col));
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
  const auto [n_elements, arg1] = (*++pc).as_i32;
  LAMAVM_EXPECT_B(arg1, ==, 0);

  tracef("barray {}", n_elements);

  const auto args = stack_args(n_elements);
  std::ranges::reverse(args);

  const box array = gc_call([&] { return boxd::Barray(box(args.data()), box(n_elements)); });

  cold::stack_pop(n_elements);
  cold::stack_push(array);

  LAMAVM_DISPATCH();
}

void op_eof() {
}

} // namespace lamavm::interpreter::cold
