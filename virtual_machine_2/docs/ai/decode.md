# Single-instruction decoder specification

Implement the cursor interface agreed in the design discussion:

- `decode_safe(std::byte*& ip, const std::byte* end, V& v)` and
  `decode_safe(const std::byte*& ip, const std::byte* end, V& v)`.
- `decode_fast(std::byte*& ip, V& v)` and
  `decode_fast(const std::byte*& ip, V& v)`.
- Each call decodes exactly one instruction. Advance `ip` to its fall-through
  address before invoking the visitor; preserve any subsequent visitor change
  to `ip`.
- Pass the visitor by reference and retain its concrete template type. Require
  callable overloads for every supported instruction layout, without inheritance
  or virtual dispatch.
- Pass a borrowed reference to the packed instruction. Input byte constness
  determines instruction constness. Mutable callbacks modify the original bytes.
- Share dispatch and instruction access between safe and fast entry points.
- Enumerate every opcode explicitly in the dispatch switch, with no default
  branch, so the compiler checks exhaustiveness. Report unsupported opcodes in
  their case group using the opcode formatter. After the switch, report unknown
  opcode values in hexadecimal, followed by `std::unreachable()`.
- Safe decoding uses `LAMAVM_EXPECT` checks before reading the opcode or a
  complete instruction. Reject null, empty, reversed, and truncated ranges.
  Range pointers must belong to the same allocation. On rejection, do not invoke
  the visitor or advance `ip`. Existing EXPECT behavior terminates the process.
- Fast decoding requires a complete, supported instruction at `ip`; it has no
  end pointer or bounds checks. Use `LAMAVM_ASSERT` for a null instruction pointer
  and unsupported opcodes. Invalid input violates its preconditions in release
  builds.
- Initially support only the instruction layouts already in `layout.hpp`:
  BINOP, BEGIN, CONST, END, EOF, STRING, JMP, CJMP, DROP, DUP, SWAP, CALL,
  LD_LOC, LD_ARG, ST_LOC, ST_ARG, LD_GLO, and ST_GLO. Reject remaining opcodes
  without callbacks. CLOSURE and its capture-span interface are deferred.
- Give each of the 13 BINOP forms in `virtual_machine/SPEC.md` its own packed,
  opcode-only struct, `ctype` check, formatter, and decoder visitor dispatch.
- Likewise, use separate packed `op_cjmp_z_t` and `op_cjmp_nz_t` layouts, each
  containing an opcode and an int32 target, with matching checks, formatters,
  and decoder visitor dispatch.
- This decoder checks instruction encoding and boundaries, not function, stack,
  target, index, or string-table semantics. EOF is decoded as one instruction;
  whole-stream termination rules belong to callers.
- Reuse the existing packed layouts and require a little-endian host, matching
  their direct representation of the bytecode specified in
  `virtual_machine/SPEC.md`. Instruction references are borrowed from the input;
  visitors must not invalidate that storage during a callback.

Validate every supported opcode through all four entry points, const-correct
callbacks, persistent visitor state, in-place operand edits, fall-through cursor
updates and visitor-directed jumps. Check malformed safe input in separate
processes, including release builds, and fast assertions in debug builds. Run
formatting, focused Clang Tidy, and the project build; report unrelated baseline
failures separately.

## Validation

- A temporary standalone harness derived opcode values and instruction lengths
  from `virtual_machine/SPEC.md` and checked all 31 supported opcodes through
  all four entry points, with and without visitor-directed jumps.
- Debug checks passed with AddressSanitizer and UndefinedBehaviorSanitizer,
  including 99 rejected-input cases. Release checks passed with `NDEBUG`,
  including 96 rejected-input cases. Failure checks also verified that the
  cursor remained unchanged.
- Formatting and focused Clang Tidy passed in debug and release configurations.
- The full release build failed on existing incomplete code in
  `source/bc/closure.cpp`: unused `size` and a missing return value.
