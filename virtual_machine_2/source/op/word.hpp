#pragma once

#include "handler.hpp"

#include "core/c.hpp"

namespace lamavm::op {

using arg_idx_t = std::int32_t; // argument
using loc_idx_t = std::int32_t; // local variable
using glo_idx_t = std::int32_t; // global variable

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
};

static_assert(ctype<word_t>);

} // namespace lamavm::op
