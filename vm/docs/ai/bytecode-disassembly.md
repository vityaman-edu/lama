# Bytecode instruction parsing and lamac dump listener

Implement the existing version-1 wire format from `src/SM.ml` using the packed
instruction structures in `lib/bc/layout.hpp`.

- Read an opcode followed by its little-endian signed 32-bit operands; dispatch
  the corresponding typed listener overload once the instruction is complete.
- Include STI (0x13), RET (0x17), and LDA (0x30–0x33), which the compiler can emit.
- CLOSURE has a target and capture count, then count pairs of an 8-bit kind and
  signed 32-bit index. Reject negative counts and kinds outside 0–3. Pass captures
  as a span valid during the callback.
- A single 0xff at an instruction boundary emits EOF and completes parsing.
  Operand bytes never act as terminators. Unknown opcodes and truncated
  instructions fail with the existing offset/scope diagnostic callback.
- Deliver the string table to listeners as a string view before imports/symbols.
- Add `lamac_ds_listener` in `lib/bc/lamac_ds.hpp` and `.cpp`, derived from
  `base_listener`, writing to a supplied ostream. Use the compiler's stack-machine
  dump spelling and formatting, resolving strings and external symbol names.
  Bytecode omits source labels, local/global names, scopes and tail-call flags;
  represent missing identities deterministically using code offsets/indices.
- Wire the executable's stdin parser to this listener; retain stderr diagnostics
  and nonzero status on parse failures.
- Check root `42.bc`, then compiler-generated regression/performance bytecode.
  Compare decoded instructions against compiler dumps after normalizing metadata
  absent from bytecode. Exercise all opcodes and truncated/invalid input.
- Keep unit coverage for signed operands containing 0xff, single-byte EOF,
  closure captures and following instruction alignment, invalid opcodes/kinds/
  capture counts, truncated streams, and exact dump text/escaping.

## Review requirements

- Source code has no comments. Name unused parameters and discard them with
  `(void)parameter` inside the function body. Use classes for types with behavior;
  reserve structs for plain data.
- String offsets are signed int32, as specified by `virtual_machine/SPEC.md`
  (Data types, Imports, Public symbols, STRING). Decode a negative external
  reference as `-(value + 1)` to avoid negating INT32_MIN.
- The standard library has `std::quoted` and debug string formatting (`{:?}`).
  Neither provides the OCaml decimal byte escapes required by lamac's dump.
  Move that exact quoting operation into `lib/support/string.hpp` and `.cpp`.
- Define `lamavm::exception` in `lib/core/exception.hpp` and `.cpp`, derived
  from `std::runtime_error`, and use it at every explicit throw site. The
  executable's existing `std::exception` handler also catches this derived type.
- The three BEGIN lists are captured-variable designations, argument names,
  and lexical scope trees (`src/SM.ml`, `insn` and `compile_fundef`). Serialized
  BEGIN stores only argument/local counts; BEGIN_CLOSURE adds a capture count.
  Keep the three list positions to match `-ds` syntax. Empty names/scopes lists
  represent unavailable metadata; Access indices in the capture list are
  positional substitutes, not recovered source designations.
- Define all opcode values in `enum class opcode : std::uint8_t` in layout.hpp.
  Instruction `code` constants and opcode fields use this enum without changing
  their serialized sizes. Read and dispatch the enum directly. The decoder's
  switch handles every enumerator, including EOF, and has no default label so
  compiler exhaustiveness warnings remain effective. Unnamed byte values fall
  through to the existing unknown-opcode diagnostic after the switch.
