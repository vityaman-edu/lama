# Cold threaded interpreter

Apply the requested iterative-to-threaded rewrite:

- Decode each unit's mutable word text once before execution. Replace handler
  IDs containing bytecode opcodes with the corresponding cold handler pointers.
  Preserve operand words and integer targets. Skip packed operand words using
  the parser's instruction layout. Retain the existing overflow, unsupported,
  and unknown-opcode diagnostics, including hexadecimal formatting.
- Keep unsupported opcodes unsupported. Decode now rejects them before execution,
  including in unreachable code. Input text has the layout produced by the unit
  parser; this change does not add operand or control-flow validation.
- At handler entry, `pc` points at its handler word. Operand reads advance it to
  the operand word. `LAMAVM_DISPATCH` advances `pc`, then invokes
  `LAMAVM_DISPATCH_JUMP`; jump dispatch calls the current handler with a required
  tail call. Taken jumps, calls, restored return addresses, and initial entry use
  jump dispatch. Conditional fall-through must also point at the next handler.
- Save CALL's next handler word as its return address. END restores the frame
  and dispatches to that address. EOF returns without dispatching, ending the
  chain and allowing the outer initialization loop to execute the next unit.
- Use parameterless internal `exec()` with the global current-unit pointer so
  its signature matches the handlers for required tail calls. Use Clang's
  `[[clang::musttail]]` spelling, supported by the configured compiler, through
  `LAMAVM_MUSTTAIL` in `core/compiler.hpp` with its paired `compiler.cpp`.

Validate release and ASan/UBSan builds, examples and imports, all supported
handler mappings and operand preservation, both conditional outcomes, invalid
handler diagnostics, and a long chain exercising bounded native-stack usage.
Check formatting, focused Clang Tidy, and whitespace without adding suppressions.

## Validation

- Release and ASan/UBSan builds passed, including the compiler macro extraction.
- All nine examples and eleven import cases passed in both configurations, with
  twenty runs per implementation and case.
- A temporary harness checked all 32 supported handler mappings, unchanged
  operand words, all 13 arithmetic handlers, both outcomes of both conditional
  branches, JMP, stack operations, LINE, and a 2,000,002-handler chain in release
  and ASan/UBSan configurations.
- Separate processes confirmed exact hexadecimal overflow, unknown-opcode, and
  unsupported-opcode diagnostics in both configurations.
- Formatting, focused Clang Tidy, and whitespace checks passed.
