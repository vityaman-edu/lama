# Execution through the instruction decoder

Apply `bc::decode_fast` to execution, as requested:

- Keep both `decode_fast` overloads and their cursor and visitor interface.
  The fast path asserts a non-null cursor, but must not compare it with or
  subtract the absent end pointer. Keep all range checks inside the safe path.
  Preserve unsupported-opcode rejection.
- Store the code span, instruction pointer, and saved return addresses as
  const bytes. Convert the unit's code span with `std::as_bytes`.
- Replace the interpreter opcode switch and `text_next` with a concrete
  visitor that traces the instruction opcode and original byte offset, then
  calls an overloaded execution function taking the decoded instruction by
  const reference. Remove interpreter stubs for unsupported instructions.
- Read operands from the packed instruction fields. The decoder advances the
  cursor before execution. Taken jumps replace it; untaken jumps retain it.
  CALL saves this fall-through cursor and END restores it using the existing
  frame representation. Preserve all stack and runtime operations, singleton
  initialization, assertions, global initialization, and EOF exit behavior.
  Copy packed operands into aligned locals before binding assertion or
  formatting references to them.
- Preserve the existing LINE no-op by adding packed `op_line_t` with opcode
  and int32 line fields, its ctype check and formatter, and decoder dispatch.
  Other unsupported instructions remain unsupported. Do not add a bytecode
  validation pass or new semantic checks.

Validate formatting, focused Clang Tidy, release and sanitizer builds, and
execution examples covering calls, locals, arguments, globals, and branches.
Use focused temporary execution checks for all arithmetic forms, both outcomes
of conditional jumps, return cursors, LINE, and string allocation. Keep unrelated
baseline failures separate.

## Validation

- Release and ASan/UBSan builds passed.
- All nine execution examples matched the reference implementations in release
  and sanitizer configurations, with 20 runs per implementation per example.
- A temporary ASan/UBSan harness checked every arithmetic opcode, both outcomes
  of both conditional jumps, JMP, CALL/BEGIN/END return cursors, argument access,
  DROP/DUP/SWAP, STRING allocation, and LINE execution. It also checked LINE
  formatting and cursor advancement through all four decoder entry points.
- Focused Clang Tidy passed in release and sanitizer configurations. Formatting
  and whitespace checks passed.
