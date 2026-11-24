#include "handler.hpp"
#include "layout.hpp"
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
#include <string_view>
#include <tuple>
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

    const box r = f(x);

    cold::stack_push(r);
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

box resolve_capture(closure_capture_t& capture) {
  auto& word = *reinterpret_cast<op::word_t*>(&capture);
  const auto kind = capture.kind;
  const auto index = capture.index;

  switch (static_cast<bc::capture_kind>(kind)) {
  case bc::capture_kind::global: {
    auto& slot = op_glo_target(op_glo_target_index(index));
    const auto offset = static_cast<std::int32_t>(&slot - stack.data());
    warm::specialize_capture(word, {.base = op::capture_t::global_base, .offset = offset});
    return slot;
  }

  case bc::capture_kind::local: {
    const auto offset = op_loc_target_index(index);
    warm::specialize_capture(word, {.base = op::capture_t::frame_base, .offset = offset});
    return op_loc_target(offset);
  }

  case bc::capture_kind::argument: {
    const auto offset = op_arg_target_index(index);
    warm::specialize_capture(word, {.base = op::capture_t::frame_base, .offset = offset});
    return fp[offset];
  }

  case bc::capture_kind::closure: {
    const auto descriptor = op_clo_capture(index);
    const auto value = op_clo_target(descriptor);
    warm::specialize_capture(word, descriptor);
    return value;
  }
  }

  fatalf("unexpected capture kind {:x}", kind);
}

} // namespace

void op_add() {
  std::ignore = op_add_t::reinterpret(pc);
  binop<boxd::Ls__Infix_43>::operator()();
  warm::specialize_op_add(pc);
  LAMAVM_DISPATCH(op_add_t);
}

void op_sub() {
  std::ignore = op_sub_t::reinterpret(pc);
  binop<boxd::Ls__Infix_45>::operator()();
  warm::specialize_op_sub(pc);
  LAMAVM_DISPATCH(op_sub_t);
}

void op_mul() {
  std::ignore = op_mul_t::reinterpret(pc);
  binop<boxd::Ls__Infix_42>::operator()();
  warm::specialize_op_mul(pc);
  LAMAVM_DISPATCH(op_mul_t);
}

void op_div() {
  std::ignore = op_div_t::reinterpret(pc);
  binop<boxd::Ls__Infix_47>::operator()();
  warm::specialize_op_div(pc);
  LAMAVM_DISPATCH(op_div_t);
}

void op_mod() {
  std::ignore = op_mod_t::reinterpret(pc);
  binop<boxd::Ls__Infix_37>::operator()();
  warm::specialize_op_mod(pc);
  LAMAVM_DISPATCH(op_mod_t);
}

void op_lt() {
  std::ignore = op_lt_t::reinterpret(pc);
  binop<boxd::Ls__Infix_60>::operator()();
  warm::specialize_op_lt(pc);
  LAMAVM_DISPATCH(op_lt_t);
}

void op_le() {
  std::ignore = op_le_t::reinterpret(pc);
  binop<boxd::Ls__Infix_6061>::operator()();
  warm::specialize_op_le(pc);
  LAMAVM_DISPATCH(op_le_t);
}

void op_gt() {
  std::ignore = op_gt_t::reinterpret(pc);
  binop<boxd::Ls__Infix_62>::operator()();
  warm::specialize_op_gt(pc);
  LAMAVM_DISPATCH(op_gt_t);
}

void op_ge() {
  std::ignore = op_ge_t::reinterpret(pc);
  binop<boxd::Ls__Infix_6261>::operator()();
  warm::specialize_op_ge(pc);
  LAMAVM_DISPATCH(op_ge_t);
}

void op_eq() {
  std::ignore = op_eq_t::reinterpret(pc);
  binop<boxd::Ls__Infix_6161>::operator()();
  warm::specialize_op_eq(pc);
  LAMAVM_DISPATCH(op_eq_t);
}

void op_ne() {
  std::ignore = op_ne_t::reinterpret(pc);
  binop<boxd::Ls__Infix_3361>::operator()();
  warm::specialize_op_ne(pc);
  LAMAVM_DISPATCH(op_ne_t);
}

void op_and() {
  std::ignore = op_and_t::reinterpret(pc);
  binop<boxd::Ls__Infix_3838>::operator()();
  warm::specialize_op_and(pc);
  LAMAVM_DISPATCH(op_and_t);
}

void op_or() {
  std::ignore = op_or_t::reinterpret(pc);
  binop<boxd::Ls__Infix_3333>::operator()();
  warm::specialize_op_or(pc);
  LAMAVM_DISPATCH(op_or_t);
}

void op_const() {
  const auto* args = op_const_t::reinterpret(pc);
  LAMAVM_EXPECT_B(args->reserved_1, ==, 0);

  const auto literal = box::checked(args->value);
  cold::stack_push(literal);

  warm::specialize_op_const(pc, literal);
  LAMAVM_DISPATCH(op_const_t);
}

void op_string() {
  const auto* args = op_string_t::reinterpret(pc);
  LAMAVM_EXPECT_B(args->reserved_1, ==, 0);

  const char* str1 = c_str(args->string_offset);
  const box str2 = boxd::Bstring(box(static_cast<const void*>(&str1)));
  cold::stack_push(str2);

  warm::specialize_op_string(pc, str1);
  LAMAVM_DISPATCH(op_string_t);
}

void op_sexp() {
  const auto* args = op_sexp_t::reinterpret(pc);
  LAMAVM_EXPECT_B(args->reserved_1, ==, 0);
  LAMAVM_EXPECT_B(args->reserved_2, ==, 0);

  LAMAVM_EXPECT_B(0, <=, args->n_fields);

  const auto hash = boxd::std_tag_hash(box(c_str(args->tag_offset)));
  cold::stack_push(hash);
  const auto values = stack_args(args->n_fields + 1);
  std::ranges::reverse(values);

  const box sexp = boxd::Bsexp(box(values.data()), box(args->n_fields + 1));

  cold::stack_pop(args->n_fields + 1);
  cold::stack_push(sexp);

  warm::specialize_op_sexp(pc, hash);
  LAMAVM_DISPATCH(op_sexp_t);
}

void op_sta() {
  std::ignore = op_sta_t::reinterpret(pc);
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

  warm::specialize_op_sta(pc);
  LAMAVM_DISPATCH(op_sta_t);
}

void op_jmp() {
  const auto* args = op_jmp_t::reinterpret(pc);
  LAMAVM_EXPECT_B(args->reserved_1, ==, 0);

  auto* destination = resolve_target(up, args->target);

  warm::specialize_op_jmp(pc, destination);

  pc = destination;
  LAMAVM_DISPATCH_JUMP();
}

void op_end() {
  std::ignore = op_end_t::reinterpret(pc);
  const box x = stack_pop_arg();
  warm::specialize_op_end(pc);

  interpreter::stack_pop_frame();
  cold::stack_push(x);
  LAMAVM_DISPATCH_JUMP();
}

void op_drop() {
  std::ignore = op_drop_t::reinterpret(pc);
  stack_pop_arg();

  warm::specialize_op_drop(pc);
  LAMAVM_DISPATCH(op_drop_t);
}

void op_dup() {
  std::ignore = op_dup_t::reinterpret(pc);
  const auto x = stack_pop_arg();
  cold::stack_push(x);
  cold::stack_push(x);

  warm::specialize_op_dup(pc);
  LAMAVM_DISPATCH(op_dup_t);
}

void op_swap() {
  std::ignore = op_swap_t::reinterpret(pc);
  const auto x = stack_pop_arg();
  const auto y = stack_pop_arg();
  cold::stack_push(x);
  cold::stack_push(y);

  warm::specialize_op_swap(pc);
  LAMAVM_DISPATCH(op_swap_t);
}

void op_elem() {
  std::ignore = op_elem_t::reinterpret(pc);
  const box index = stack_pop_arg();
  const box aggregate = stack_pop_arg();

  tracef("elem {}[{}]", unbox<void*>(aggregate), unbox<std::int64_t>(index));
  cold::stack_push(boxd::Belem(aggregate, index));

  warm::specialize_op_elem(pc);
  LAMAVM_DISPATCH(op_elem_t);
}

void op_ld_glo() {
  const auto* args = op_ld_glo_t::reinterpret(pc);
  LAMAVM_EXPECT_B(args->reserved_1, ==, 0);

  const auto target_index = op_glo_target_index(args->index);
  auto& target = op_glo_target(target_index);

  tracef("ld_glo {} = {}", target_index.index, unbox<std::int64_t>(target));
  cold::stack_push(target);

  warm::specialize_op_ld_glo(pc, target);
  LAMAVM_DISPATCH(op_ld_glo_t);
}

void op_ld_loc() {
  const auto* args = op_ld_loc_t::reinterpret(pc);
  LAMAVM_EXPECT_B(args->reserved_1, ==, 0);

  const auto offset = op_loc_target_index(args->index);
  const auto loc = op_loc_target(offset);
  tracef("ld_loc {} = {}", args->index, unbox<std::int64_t>(loc));
  cold::stack_push(loc);

  warm::specialize_op_ld_loc(pc, offset);
  LAMAVM_DISPATCH(op_ld_loc_t);
}

void op_ld_arg() {
  const auto* args = op_ld_arg_t::reinterpret(pc);
  LAMAVM_EXPECT_B(args->reserved_1, ==, 0);

  const auto offset = op_arg_target_index(args->index);
  const auto arg = fp[offset];
  tracef("ld_arg {} = {}", args->index, unbox<std::int64_t>(arg));
  cold::stack_push(arg);

  warm::specialize_op_ld_arg(pc, offset);
  LAMAVM_DISPATCH(op_ld_arg_t);
}

void op_ld_clo() {
  const auto* args = op_ld_clo_t::reinterpret(pc);
  LAMAVM_EXPECT_B(args->reserved_1, ==, 0);

  const auto capture = op_clo_capture(args->index);
  const auto arg = op_clo_target(capture);
  tracef("ld_clo {} = {}", args->index, unbox<std::int64_t>(arg));
  cold::stack_push(arg);

  warm::specialize_op_ld_clo(pc, capture.base, capture.offset);
  LAMAVM_DISPATCH(op_ld_clo_t);
}

void op_st_glo() {
  const auto* args = op_st_glo_t::reinterpret(pc);
  LAMAVM_EXPECT_B(args->reserved_1, ==, 0);

  const auto target_index = op_glo_target_index(args->index);
  auto& target = op_glo_target(target_index);

  const auto value = stack_pop_arg();
  cold::stack_push(value);

  tracef("sd_glo {} := {}", target_index.index, unbox<std::int64_t>(value));
  target = value;

  warm::specialize_op_st_glo(pc, target);
  LAMAVM_DISPATCH(op_st_glo_t);
}

void op_st_loc() {
  const auto* args = op_st_loc_t::reinterpret(pc);
  LAMAVM_EXPECT_B(args->reserved_1, ==, 0);

  const auto value = stack_pop_arg();
  cold::stack_push(value);

  const auto offset = op_loc_target_index(args->index);
  auto& loc = op_loc_target(offset);
  tracef("st_loc {} := {}", args->index, unbox<std::int64_t>(value));
  loc = value;

  warm::specialize_op_st_loc(pc, offset);
  LAMAVM_DISPATCH(op_st_loc_t);
}

void op_st_arg() {
  const auto* args = op_st_arg_t::reinterpret(pc);
  LAMAVM_EXPECT_B(args->reserved_1, ==, 0);

  const auto value = stack_pop_arg();
  cold::stack_push(value);

  const auto offset = op_arg_target_index(args->index);
  tracef("st_arg {} := {}", args->index, unbox<std::int64_t>(value));
  fp[offset] = value;

  warm::specialize_op_st_arg(pc, offset);
  LAMAVM_DISPATCH(op_st_arg_t);
}

void op_st_clo() {
  const auto* args = op_st_clo_t::reinterpret(pc);
  LAMAVM_EXPECT_B(args->reserved_1, ==, 0);

  const auto value = stack_pop_arg();
  cold::stack_push(value);

  const auto capture = op_clo_capture(args->index);
  auto& arg = op_clo_target(capture);
  tracef("st_clo {} := {}", args->index, unbox<std::int64_t>(value));
  arg = value;

  warm::specialize_op_st_clo(pc, capture.base, capture.offset);
  LAMAVM_DISPATCH(op_st_clo_t);
}

void op_cjmp_z() {
  const auto* args = op_cjmp_z_t::reinterpret(pc);
  LAMAVM_EXPECT_B(args->reserved_1, ==, 0);

  auto* destination = resolve_target(up, args->target);
  const auto value = stack_pop_arg();
  const auto is_taken = unbox<std::int64_t>(value) == 0;

  warm::specialize_op_cjmp_z(pc, destination);

  pc = is_taken ? destination : pc + word_count<op_cjmp_z_t>();
  LAMAVM_DISPATCH_JUMP();
}

void op_cjmp_nz() {
  const auto* args = op_cjmp_nz_t::reinterpret(pc);
  LAMAVM_EXPECT_B(args->reserved_1, ==, 0);

  auto* destination = resolve_target(up, args->target);
  const auto value = stack_pop_arg();
  const auto is_taken = unbox<std::int64_t>(value) != 0;

  warm::specialize_op_cjmp_nz(pc, destination);

  pc = is_taken ? destination : pc + word_count<op_cjmp_nz_t>();
  LAMAVM_DISPATCH_JUMP();
}

void op_begin() {
  const auto* args = op_begin_t::reinterpret(pc);
  stack_init_locals(args->n_locals);

  warm::specialize_op_begin(pc);
  LAMAVM_DISPATCH(op_begin_t);
}

void op_begin_closure() {
  const auto* args = op_begin_closure_t::reinterpret(pc);
  LAMAVM_EXPECT_B(args->reserved_1, ==, 0);

  LAMAVM_EXPECT_B(args->n_args, ==, stack_n_args());
  const auto closure = closure_payload<hard>(*stack_frame_closure());
  LAMAVM_EXPECT_B(args->n_captures, ==, std::ssize(closure) - 2);

  stack_init_locals(args->n_locals);

  warm::specialize_op_begin_closure(pc);
  LAMAVM_DISPATCH(op_begin_closure_t);
}

void op_closure() {
  const auto args = op_closure_view_t::reinterpret(pc);
  const auto& [header, captures] = args;
  LAMAVM_EXPECT_B(header->reserved_1, ==, 0);
  LAMAVM_EXPECT_B(header->reserved_2, ==, 0);

  auto target = header->target;

  const auto* target_up = up;
  if (target < 0) {
    const auto& s = find_symbol(c_str(-(target + 1)));
    LAMAVM_EXPECT_B(s.kind, ==, bc::symbol::kind::function);

    target_up = units.data() + s.uidx;
    target = s.offset;
  }

  auto* entry = resolve_target(target_up, target);

  for (auto& capture : std::views::reverse(captures)) {
    cold::stack_push(resolve_capture(capture));
  }

  cold::stack_push(box(target_up));
  cold::stack_push(box(target));

  const box cj = boxd::Bclosure(box(static_cast<box*>(sp)), box(1 + header->n_captured));

  cold::stack_pop(std::int64_t{2} + header->n_captured);

  cold::stack_push(cj);

  warm::specialize_op_closure(pc, entry, target_up);

  pc += word_count(args);
  LAMAVM_DISPATCH_JUMP();
}

void op_callc() {
  const auto* args = op_callc_t::reinterpret(pc);
  LAMAVM_EXPECT_B(args->reserved_1, ==, 0);

  LAMAVM_EXPECT_B(0, <=, args->n_args);
  const auto values = stack_args(std::int64_t{args->n_args} + 1);
  const auto cj = closure_payload<hard>(values.back());
  const auto target = unbox<std::int32_t>(cj[0]);
  const auto* target_up = unbox<const uctx*>(cj[1]);
  auto* entry = resolve_target(target_up, target);

  cold::stack_push_frame({
    .n_args = args->n_args,
    .fp = fp,
    .pc = pc + word_count<op_callc_t>(),
    .sp = sp + args->n_args + 1,
    .up = up,
  });

  warm::specialize_op_callc(pc);

  up = target_up;
  pc = entry;
  LAMAVM_DISPATCH_JUMP();
}

void op_call() {
  const auto* args = op_call_t::reinterpret(pc);
  LAMAVM_EXPECT_B(args->reserved_1, ==, 0);
  LAMAVM_EXPECT_B(args->reserved_2, ==, 0);

  auto target = args->target;
  const auto values = stack_args(args->n_args);

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
        LAMAVM_EXPECT_B(s.head, ==, args->n_args);
        break;
      case std_fn_symbol::tail_kind::vararg:
        LAMAVM_EXPECT_B(s.head, <=, args->n_args);
        break;
      }

      const box r = s.fn(values);
      cold::stack_pop(args->n_args);
      cold::stack_push(r);

      warm::specialize_op_call_std(pc, s.fn);
      LAMAVM_DISPATCH(op_call_t);
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
  cold::stack_push_frame({
    .n_args = args->n_args,
    .fp = fp,
    .pc = pc + word_count<op_call_t>(),
    .sp = sp + args->n_args,
    .up = up,
  });

  warm::specialize_op_call(pc, entry, callee, up);

  up = callee;
  pc = entry;
  LAMAVM_DISPATCH_JUMP();
}

void op_tag() {
  const auto* args = op_tag_t::reinterpret(pc);
  LAMAVM_EXPECT_B(args->reserved_1, ==, 0);
  LAMAVM_EXPECT_B(args->reserved_2, ==, 0);

  const box tag_hash = boxd::std_tag_hash(box(c_str(args->tag_offset)));
  const box value = stack_pop_arg();
  cold::stack_push(boxd::Btag(value, tag_hash, box(args->n_fields)));

  warm::specialize_op_tag(pc, tag_hash, box(args->n_fields));
  LAMAVM_DISPATCH(op_tag_t);
}

void op_array() {
  const auto* args = op_array_t::reinterpret(pc);
  LAMAVM_EXPECT_B(args->reserved_1, ==, 0);

  const box value = stack_pop_arg();
  tracef("array {}", args->n_elements);
  cold::stack_push(boxd::Barray_patt(value, box(args->n_elements)));

  warm::specialize_op_array(pc, box(args->n_elements));
  LAMAVM_DISPATCH(op_array_t);
}

void op_fail() {
  const auto* args = op_fail_t::reinterpret(pc);
  const box value = stack_pop_arg();
  tracef("fail {}:{}:{}", up->name, args->line, args->column);
  boxd::Bmatch_failure(value, box(up->name), box(args->line), box(args->column));
}

void op_fail_keep() {
  const auto* args = op_fail_keep_t::reinterpret(pc);
  tracef("fail_keep {}:{}:{}", up->name, args->line, args->column);
  boxd::Bmatch_failure(stack_args(1).front(), box(up->name), box(args->line), box(args->column));
}

void op_line() {
  std::ignore = op_line_t::reinterpret(pc);

  warm::specialize_op_line(pc);
  LAMAVM_DISPATCH(op_line_t);
}

void op_patt_str_cmp() {
  std::ignore = op_patt_str_cmp_t::reinterpret(pc);
  binop<boxd::Bstring_patt>::operator()();

  warm::specialize_op_patt_str_cmp(pc);
  LAMAVM_DISPATCH(op_patt_str_cmp_t);
}

void op_patt_string() {
  std::ignore = op_patt_string_t::reinterpret(pc);
  unop<boxd::Bstring_tag_patt>::operator()();

  warm::specialize_op_patt_string(pc);
  LAMAVM_DISPATCH(op_patt_string_t);
}

void op_patt_array() {
  std::ignore = op_patt_array_t::reinterpret(pc);
  unop<boxd::Barray_tag_patt>::operator()();

  warm::specialize_op_patt_array(pc);
  LAMAVM_DISPATCH(op_patt_array_t);
}

void op_patt_sexp() {
  std::ignore = op_patt_sexp_t::reinterpret(pc);
  unop<boxd::Bsexp_tag_patt>::operator()();

  warm::specialize_op_patt_sexp(pc);
  LAMAVM_DISPATCH(op_patt_sexp_t);
}

void op_patt_boxed() {
  std::ignore = op_patt_boxed_t::reinterpret(pc);
  unop<boxd::Bboxed_patt>::operator()();

  warm::specialize_op_patt_boxed(pc);
  LAMAVM_DISPATCH(op_patt_boxed_t);
}

void op_patt_unboxed() {
  std::ignore = op_patt_unboxed_t::reinterpret(pc);
  unop<boxd::Bunboxed_patt>::operator()();

  warm::specialize_op_patt_unboxed(pc);
  LAMAVM_DISPATCH(op_patt_unboxed_t);
}

void op_patt_closure() {
  std::ignore = op_patt_closure_t::reinterpret(pc);
  unop<boxd::Bclosure_tag_patt>::operator()();

  warm::specialize_op_patt_closure(pc);
  LAMAVM_DISPATCH(op_patt_closure_t);
}

void op_barray() {
  const auto* args = op_barray_t::reinterpret(pc);
  LAMAVM_EXPECT_B(args->reserved_1, ==, 0);

  tracef("barray {}", args->n_elements);

  const auto values = stack_args(args->n_elements);
  std::ranges::reverse(values);

  const box array = boxd::Barray(box(values.data()), box(args->n_elements));

  cold::stack_pop(args->n_elements);
  cold::stack_push(array);

  warm::specialize_op_barray(pc);
  LAMAVM_DISPATCH(op_barray_t);
}

void op_eof() {
  std::ignore = op_eof_t::reinterpret(pc);
}

} // namespace lamavm::interpreter::cold
