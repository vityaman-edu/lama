#include "handler.hpp"
#include "state.hpp"

#include "interpreter/runtime/box.hpp"
#include "interpreter/runtime/closure.hpp"
#include "interpreter/runtime/state.hpp"
#include "interpreter/std/std.g.hpp"
#include "interpreter/std/std.hpp"
#include "interpreter/warm/specialization.hpp"

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
#include <limits>
#include <ranges>
#include <span>
#include <string_view>
#include <vector>

namespace lamavm::interpreter::cold {

namespace {

op::word_t* resolve_target(const uctx* unit, std::int32_t target) {
  LAMAVM_EXPECT_BB(0, <=, target, <, std::ssize(unit->text));

  return unit->text.data() + target;
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

std::int32_t op_loc_target_index(std::int32_t index) {
  LAMAVM_EXPECT_BB(0, <=, index, <=, fp - stack.data() - frame::size);

  return -static_cast<std::int32_t>(frame::size) - index;
}

box& op_loc_target(std::int64_t index) {
  return fp[index];
}

std::int32_t op_arg_target_index(std::int32_t index) {
  const auto n_args = stack_n_args();
  LAMAVM_EXPECT_BB(0, <=, index, <, n_args);

  return n_args - index;
}

op::capture_t op_clo_capture(std::int32_t index) {
  LAMAVM_EXPECT_BB(0, <=, index, <=, std::numeric_limits<std::int32_t>::max() - 2);

  return {.base = static_cast<std::int32_t>(stack_frame_closure() - fp), .offset = index + 2};
}

box& op_clo_target(op::capture_t capture) {
  const auto closure = closure_payload<hard>(fp[capture.base]);
  LAMAVM_EXPECT_B(capture.offset, <, std::ssize(closure));

  return closure[capture.offset];
}

box resolve_capture(op::word_t& capture) {
  const auto [kind, index] = capture.as_i32;
  switch (static_cast<bc::capture_kind>(kind)) {
  case bc::capture_kind::global: {
    auto& slot = op_glo_target(op_glo_target_index(index));
    const auto offset = static_cast<std::int32_t>(&slot - stack.data());
    warm::specialize_capture(capture, {.base = op::capture_t::global_base, .offset = offset});
    return slot;
  }

  case bc::capture_kind::local: {
    const auto offset = op_loc_target_index(index);
    warm::specialize_capture(capture, {.base = op::capture_t::frame_base, .offset = offset});
    return op_loc_target(offset);
  }

  case bc::capture_kind::argument: {
    const auto offset = op_arg_target_index(index);
    warm::specialize_capture(capture, {.base = op::capture_t::frame_base, .offset = offset});
    return fp[offset];
  }

  case bc::capture_kind::closure: {
    const auto descriptor = op_clo_capture(index);
    const auto value = op_clo_target(descriptor);
    warm::specialize_capture(capture, descriptor);
    return value;
  }
  }

  fatalf("unexpected capture kind {:x}", kind);
}

} // namespace

void op_add() {
  auto* ip = pc;
  binop<boxd::Ls__Infix_43>::operator()();
  warm::specialize_op_add(ip);
  LAMAVM_DISPATCH();
}

void op_sub() {
  auto* ip = pc;
  binop<boxd::Ls__Infix_45>::operator()();
  warm::specialize_op_sub(ip);
  LAMAVM_DISPATCH();
}

void op_mul() {
  auto* ip = pc;
  binop<boxd::Ls__Infix_42>::operator()();
  warm::specialize_op_mul(ip);
  LAMAVM_DISPATCH();
}

void op_div() {
  auto* ip = pc;
  binop<boxd::Ls__Infix_47>::operator()();
  warm::specialize_op_div(ip);
  LAMAVM_DISPATCH();
}

void op_mod() {
  auto* ip = pc;
  binop<boxd::Ls__Infix_37>::operator()();
  warm::specialize_op_mod(ip);
  LAMAVM_DISPATCH();
}

void op_lt() {
  auto* ip = pc;
  binop<boxd::Ls__Infix_60>::operator()();
  warm::specialize_op_lt(ip);
  LAMAVM_DISPATCH();
}

void op_le() {
  auto* ip = pc;
  binop<boxd::Ls__Infix_6061>::operator()();
  warm::specialize_op_le(ip);
  LAMAVM_DISPATCH();
}

void op_gt() {
  auto* ip = pc;
  binop<boxd::Ls__Infix_62>::operator()();
  warm::specialize_op_gt(ip);
  LAMAVM_DISPATCH();
}

void op_ge() {
  auto* ip = pc;
  binop<boxd::Ls__Infix_6261>::operator()();
  warm::specialize_op_ge(ip);
  LAMAVM_DISPATCH();
}

void op_eq() {
  auto* ip = pc;
  binop<boxd::Ls__Infix_6161>::operator()();
  warm::specialize_op_eq(ip);
  LAMAVM_DISPATCH();
}

void op_ne() {
  auto* ip = pc;
  binop<boxd::Ls__Infix_3361>::operator()();
  warm::specialize_op_ne(ip);
  LAMAVM_DISPATCH();
}

void op_and() {
  auto* ip = pc;
  binop<boxd::Ls__Infix_3838>::operator()();
  warm::specialize_op_and(ip);
  LAMAVM_DISPATCH();
}

void op_or() {
  auto* ip = pc;
  binop<boxd::Ls__Infix_3333>::operator()();
  warm::specialize_op_or(ip);
  LAMAVM_DISPATCH();
}

void op_const() {
  auto* ip = pc;
  const auto [value, arg1] = (*++pc).as_i32;
  LAMAVM_EXPECT_B(arg1, ==, 0);

  const auto literal = box::checked(value);
  cold::stack_push(literal);

  warm::specialize_op_const(ip, literal);
  LAMAVM_DISPATCH();
}

void op_string() {
  auto* ip = pc;
  const auto [string_offset, arg1] = (*++pc).as_i32;
  LAMAVM_EXPECT_B(arg1, ==, 0);

  const char* str1 = c_str(string_offset);
  const box str2 = boxd::Bstring(box(static_cast<const void*>(&str1)));
  cold::stack_push(str2);

  warm::specialize_op_string(ip, str1);
  LAMAVM_DISPATCH();
}

void op_sexp() {
  auto* ip = pc;
  const auto [tag_offset, arg10] = (*++pc).as_i32;
  LAMAVM_EXPECT_B(arg10, ==, 0);

  const auto [n_fields, arg11] = (*++pc).as_i32;
  LAMAVM_EXPECT_B(arg11, ==, 0);
  LAMAVM_EXPECT_B(0, <=, n_fields);

  const auto hash = boxd::std_tag_hash(box(c_str(tag_offset)));
  cold::stack_push(hash);
  const auto args = stack_args(n_fields + 1);
  std::ranges::reverse(args);

  const box sexp = boxd::Bsexp(box(args.data()), box(n_fields + 1));

  cold::stack_pop(n_fields + 1);
  cold::stack_push(sexp);

  warm::specialize_op_sexp(ip, hash);
  LAMAVM_DISPATCH();
}

void op_sta() {
  auto* ip = pc;
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

  warm::specialize_op_sta(ip);
  LAMAVM_DISPATCH();
}

void op_jmp() {
  auto* ip = pc;
  const auto [target, arg1] = (*++pc).as_i32;
  LAMAVM_EXPECT_B(arg1, ==, 0);

  pc = resolve_target(up, target);

  warm::specialize_op_jmp(ip, pc);
  LAMAVM_DISPATCH_JUMP();
}

void op_end() {
  auto* ip = pc;
  const box x = stack_pop_arg();
  interpreter::stack_pop_frame();
  cold::stack_push(x);

  warm::specialize_op_end(ip);
  LAMAVM_DISPATCH_JUMP();
}

void op_drop() {
  auto* ip = pc;
  stack_pop_arg();

  warm::specialize_op_drop(ip);
  LAMAVM_DISPATCH();
}

void op_dup() {
  auto* ip = pc;
  const auto x = stack_pop_arg();
  cold::stack_push(x);
  cold::stack_push(x);

  warm::specialize_op_dup(ip);
  LAMAVM_DISPATCH();
}

void op_swap() {
  auto* ip = pc;
  const auto x = stack_pop_arg();
  const auto y = stack_pop_arg();
  cold::stack_push(x);
  cold::stack_push(y);

  warm::specialize_op_swap(ip);
  LAMAVM_DISPATCH();
}

void op_elem() {
  auto* ip = pc;
  const box index = stack_pop_arg();
  const box aggregate = stack_pop_arg();

  tracef("elem {}[{}]", unbox<void*>(aggregate), unbox<std::int64_t>(index));
  cold::stack_push(boxd::Belem(aggregate, index));

  warm::specialize_op_elem(ip);
  LAMAVM_DISPATCH();
}

void op_ld_glo() {
  auto* ip = pc;
  const auto [operand, arg1] = (*++pc).as_i32;
  LAMAVM_EXPECT_B(arg1, ==, 0);

  const auto target_index = op_glo_target_index(operand);
  auto& target = op_glo_target(target_index);

  tracef("ld_glo {} = {}", target_index.index, unbox<std::int64_t>(target));
  cold::stack_push(target);

  warm::specialize_op_ld_glo(ip, target);
  LAMAVM_DISPATCH();
}

void op_ld_loc() {
  auto* ip = pc;
  const auto [index, arg1] = (*++pc).as_i32;
  LAMAVM_EXPECT_B(arg1, ==, 0);

  const auto offset = op_loc_target_index(index);
  const auto loc = op_loc_target(offset);
  tracef("ld_loc {} = {}", index, unbox<std::int64_t>(loc));
  cold::stack_push(loc);

  warm::specialize_op_ld_loc(ip, offset);
  LAMAVM_DISPATCH();
}

void op_ld_arg() {
  auto* ip = pc;
  const auto [index, arg1] = (*++pc).as_i32;
  LAMAVM_EXPECT_B(arg1, ==, 0);

  const auto offset = op_arg_target_index(index);
  const auto arg = fp[offset];
  tracef("ld_arg {} = {}", index, unbox<std::int64_t>(arg));
  cold::stack_push(arg);

  warm::specialize_op_ld_arg(ip, offset);
  LAMAVM_DISPATCH();
}

void op_ld_clo() {
  auto* ip = pc;
  const auto [index, arg1] = (*++pc).as_i32;
  LAMAVM_EXPECT_B(arg1, ==, 0);

  const auto capture = op_clo_capture(index);
  const auto arg = op_clo_target(capture);
  tracef("ld_clo {} = {}", index, unbox<std::int64_t>(arg));
  cold::stack_push(arg);

  warm::specialize_op_ld_clo(ip, capture.base, capture.offset);
  LAMAVM_DISPATCH();
}

void op_st_glo() {
  auto* ip = pc;
  const auto [operand, arg1] = (*++pc).as_i32;
  LAMAVM_EXPECT_B(arg1, ==, 0);

  const auto target_index = op_glo_target_index(operand);
  auto& target = op_glo_target(target_index);

  const auto value = stack_pop_arg();
  cold::stack_push(value);

  tracef("sd_glo {} := {}", target_index.index, unbox<std::int64_t>(value));
  target = value;

  warm::specialize_op_st_glo(ip, target);
  LAMAVM_DISPATCH();
}

void op_st_loc() {
  auto* ip = pc;
  const auto [index, arg1] = (*++pc).as_i32;
  LAMAVM_EXPECT_B(arg1, ==, 0);

  const auto value = stack_pop_arg();
  cold::stack_push(value);

  const auto offset = op_loc_target_index(index);
  auto& loc = op_loc_target(offset);
  tracef("st_loc {} := {}", index, unbox<std::int64_t>(value));
  loc = value;

  warm::specialize_op_st_loc(ip, offset);
  LAMAVM_DISPATCH();
}

void op_st_arg() {
  auto* ip = pc;
  const auto [index, arg1] = (*++pc).as_i32;
  LAMAVM_EXPECT_B(arg1, ==, 0);

  const auto value = stack_pop_arg();
  cold::stack_push(value);

  const auto offset = op_arg_target_index(index);
  tracef("st_arg {} := {}", index, unbox<std::int64_t>(value));
  fp[offset] = value;

  warm::specialize_op_st_arg(ip, offset);
  LAMAVM_DISPATCH();
}

void op_st_clo() {
  auto* ip = pc;
  const auto [index, arg1] = (*++pc).as_i32;
  LAMAVM_EXPECT_B(arg1, ==, 0);

  const auto value = stack_pop_arg();
  cold::stack_push(value);

  const auto capture = op_clo_capture(index);
  auto& arg = op_clo_target(capture);
  tracef("st_clo {} := {}", index, unbox<std::int64_t>(value));
  arg = value;

  warm::specialize_op_st_clo(ip, capture.base, capture.offset);
  LAMAVM_DISPATCH();
}

void op_cjmp_z() {
  auto* ip = pc;
  const auto [target, arg1] = (*++pc).as_i32;
  LAMAVM_EXPECT_B(arg1, ==, 0);

  auto* destination = resolve_target(up, target);
  const auto value = stack_pop_arg();
  pc = unbox<std::int64_t>(value) == 0 ? destination : ip + 2;

  warm::specialize_op_cjmp_z(ip, destination);
  LAMAVM_DISPATCH_JUMP();
}

void op_cjmp_nz() {
  auto* ip = pc;
  const auto [target, arg1] = (*++pc).as_i32;
  LAMAVM_EXPECT_B(arg1, ==, 0);

  auto* destination = resolve_target(up, target);
  const auto value = stack_pop_arg();
  pc = unbox<std::int64_t>(value) != 0 ? destination : ip + 2;

  warm::specialize_op_cjmp_nz(ip, destination);
  LAMAVM_DISPATCH_JUMP();
}

void op_begin() {
  auto* ip = pc;
  stack_init_locals((*++pc).as_i32.arg1);

  warm::specialize_op_begin(ip);
  LAMAVM_DISPATCH();
}

void op_begin_closure() {
  auto* ip = pc;
  const auto [n_args, n_locals] = (*++pc).as_i32;

  const auto [n_captures, arg1] = (*++pc).as_i32;
  LAMAVM_EXPECT_B(arg1, ==, 0);

  LAMAVM_EXPECT_B(n_args, ==, stack_n_args());
  const auto closure = closure_payload<hard>(*stack_frame_closure());
  LAMAVM_EXPECT_B(n_captures, ==, std::ssize(closure) - 2);

  stack_init_locals(n_locals);

  warm::specialize_op_begin_closure(ip);
  LAMAVM_DISPATCH();
}

void op_closure() {
  auto* ip = pc;
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

  auto* entry = resolve_target(target_up, target);
  const std::span<op::word_t> captured(pc + 1, static_cast<std::size_t>(n_captured));
  pc += n_captured;

  for (auto& capture : std::views::reverse(captured)) {
    cold::stack_push(resolve_capture(capture));
  }

  cold::stack_push(box(target_up));
  cold::stack_push(box(target));

  const box cj = boxd::Bclosure(box(sp), box(1 + n_captured));

  cold::stack_pop(std::int64_t{2} + n_captured);

  cold::stack_push(cj);

  warm::specialize_op_closure(ip, entry, target_up);
  LAMAVM_DISPATCH();
}

void op_callc() {
  auto* ip = pc;
  const auto [n_args, arg1] = (*++pc).as_i32;
  LAMAVM_EXPECT_B(arg1, ==, 0);

  LAMAVM_EXPECT_B(0, <=, n_args);
  const auto args = stack_args(std::int64_t{n_args} + 1);
  const auto cj = closure_payload<hard>(args.back());
  const auto target = unbox<std::int32_t>(cj[0]);
  const auto* target_up = unbox<const uctx*>(cj[1]);
  auto* entry = resolve_target(target_up, target);

  cold::stack_push_frame({
    .n_args = n_args,
    .fp = fp,
    .pc = pc + 1,
    .sp = sp + n_args + 1,
    .up = up,
  });

  up = target_up;
  pc = entry;
  warm::specialize_op_callc(ip);
  LAMAVM_DISPATCH_JUMP();
}

void op_call() {
  auto* ip = pc;
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

      const box r = s.fn(args);
      cold::stack_pop(n_args);
      cold::stack_push(r);

      warm::specialize_op_call_std(ip, s.fn);
      LAMAVM_DISPATCH();
    }
  }

  const auto* callee = up;
  if (target < 0) {
    const auto& s = find_symbol(name);
    LAMAVM_EXPECT_B(s.kind, ==, bc::symbol::kind::function);

    callee = units.data() + s.uidx;

    target = s.offset;
  }

  auto* entry = resolve_target(callee, target);
  const auto* caller = up;
  cold::stack_push_frame({
    .n_args = n_args,
    .fp = fp,
    .pc = pc + 1,
    .sp = sp + n_args,
    .up = up,
  });

  up = callee;
  pc = entry;
  warm::specialize_op_call(ip, entry, callee, caller);
  LAMAVM_DISPATCH_JUMP();
}

void op_tag() {
  auto* ip = pc;
  const auto [tag_offset, arg10] = (*++pc).as_i32;
  LAMAVM_EXPECT_B(arg10, ==, 0);

  const auto [n_fields, arg11] = (*++pc).as_i32;
  LAMAVM_EXPECT_B(arg11, ==, 0);

  const box tag_hash = boxd::std_tag_hash(box(c_str(tag_offset)));
  const box value = stack_pop_arg();
  cold::stack_push(boxd::Btag(value, tag_hash, box(n_fields)));

  warm::specialize_op_tag(ip, tag_hash, box(n_fields));
  LAMAVM_DISPATCH();
}

void op_array() {
  auto* ip = pc;
  const auto [n_elements, arg1] = (*++pc).as_i32;
  LAMAVM_EXPECT_B(arg1, ==, 0);

  const box value = stack_pop_arg();
  tracef("array {}", n_elements);
  cold::stack_push(boxd::Barray_patt(value, box(n_elements)));

  warm::specialize_op_array(ip, box(n_elements));
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
  auto* ip = pc;
  ++pc;

  warm::specialize_op_line(ip);
  LAMAVM_DISPATCH();
}

void op_patt_str_cmp() {
  auto* ip = pc;
  binop<boxd::Bstring_patt>::operator()();

  warm::specialize_op_patt_str_cmp(ip);
  LAMAVM_DISPATCH();
}

void op_patt_string() {
  auto* ip = pc;
  unop<boxd::Bstring_tag_patt>::operator()();

  warm::specialize_op_patt_string(ip);
  LAMAVM_DISPATCH();
}

void op_patt_array() {
  auto* ip = pc;
  unop<boxd::Barray_tag_patt>::operator()();

  warm::specialize_op_patt_array(ip);
  LAMAVM_DISPATCH();
}

void op_patt_sexp() {
  auto* ip = pc;
  unop<boxd::Bsexp_tag_patt>::operator()();

  warm::specialize_op_patt_sexp(ip);
  LAMAVM_DISPATCH();
}

void op_patt_boxed() {
  auto* ip = pc;
  unop<boxd::Bboxed_patt>::operator()();

  warm::specialize_op_patt_boxed(ip);
  LAMAVM_DISPATCH();
}

void op_patt_unboxed() {
  auto* ip = pc;
  unop<boxd::Bunboxed_patt>::operator()();

  warm::specialize_op_patt_unboxed(ip);
  LAMAVM_DISPATCH();
}

void op_patt_closure() {
  auto* ip = pc;
  unop<boxd::Bclosure_tag_patt>::operator()();

  warm::specialize_op_patt_closure(ip);
  LAMAVM_DISPATCH();
}

void op_barray() {
  auto* ip = pc;
  const auto [n_elements, arg1] = (*++pc).as_i32;
  LAMAVM_EXPECT_B(arg1, ==, 0);

  tracef("barray {}", n_elements);

  const auto args = stack_args(n_elements);
  std::ranges::reverse(args);

  const box array = boxd::Barray(box(args.data()), box(n_elements));

  cold::stack_pop(n_elements);
  cold::stack_push(array);

  warm::specialize_op_barray(ip);
  LAMAVM_DISPATCH();
}

void op_eof() {
}

} // namespace lamavm::interpreter::cold
