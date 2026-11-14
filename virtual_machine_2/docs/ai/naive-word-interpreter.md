# Naive word interpreter

Apply the requested replacement of packed-byte decoding in
`source/interpreter/loop.cpp` with direct `op::word_t` execution.

- Fetch the handler with `(*ip++).as_handler_id` and dispatch using a switch
  over `op::handler_id_t`. Handler IDs retain their serialized opcode values.
- Keep a separate no-argument function for each currently supported operation.
  Unsupported handlers continue to fail; do not implement additional operations.
- Fetch operand words with `*ip++` and access `as_i32.arg0` and `as_i32.arg1`.
  One-operand instructions use `arg0`; CALL uses target and argument count;
  BEGIN uses argument count and local count. LINE consumes one operand word.
- Keep string references as string-table offsets and code references as integer
  offsets into the word array. Do not install pointers into operand words.
  Convert serialized byte offsets to word offsets during `unit::parse` as
  specified below.
- Consume branch operands before testing conditions. CALL saves the cursor after
  its operand word, and END restores it through the existing stack frame.
- Preserve runtime calls, stack operations, singleton initialization, globals,
  initial frame, and EOF behavior. Check EOF through the final word's handler ID.

Validate with focused formatting and Clang Tidy, release and sanitizer builds,
and temporary word-array execution checks for operands, branches, and calls.

## Parser target conversion

Apply the requested two-pass conversion, following the reference converter's
`bc_to_insn_map`:

- Build a temporary `std::vector<std::int32_t> word_by_bc`, indexed by byte
  offset relative to the code section. Record the current `u.text` index at
  each opcode boundary, including EOF; use `-1` for non-opcode bytes.
- After parsing, walk instructions in `u.text`, respecting one- and two-word
  operand layouts and variable-length CLOSURE capture lists.
- Rewrite JMP, CJMP_Z, CJMP_NZ, and nonnegative CALL/CLOSURE targets through
  `word_by_bc`. Retain negative external references for later linking.
- Translate public function offsets through the same map. Preserve global
  indices, string offsets, argument counts, and capture designations.
- Require translated targets to be in range and to identify opcode boundaries.
  Keep the results as integer indices; do not install pointers.

## Validation

- Release and ASan/UBSan builds passed.
- A temporary ASan/UBSan word-array harness passed all supported arithmetic
  operations, taken and untaken conditional branches, JMP, LINE, stack
  manipulation, globals, locals, arguments, CALL/BEGIN/END return cursors,
  STRING allocation, and unsupported-handler rejection.
- A serialized straight-line BEGIN/CONST/END/EOF fixture passed through parsing
  and execution in both build configurations.
- Formatting and focused whitespace checks passed.
- Focused Clang Tidy reports 14 `cppcoreguidelines-pro-type-union-access`
  diagnostics for the requested direct union accesses. No rules were disabled.
- The initial interpreter validation preceded parser target conversion; parser
  conversion is validated separately below.

## Parser conversion validation

- Release and ASan/UBSan builds passed.
- All nine examples matched the reference implementations in both builds,
  with 20 runs per implementation per example.
- A temporary ASan/UBSan parser harness checked all instruction layouts,
  forward and backward targets, EOF targets, public function offsets,
  unchanged globals and external references, and variable-length closure
  capture lists. Negative jumps, out-of-range targets, and targets inside
  operands were rejected.
- Formatting and focused whitespace checks passed.
- Focused parser Clang Tidy reports three existing diagnostics:
  `hicpp-member-init` on the read temporary and `misc-include-cleaner` for
  `handler_id_t` and `char_traits`. No rules were disabled.
