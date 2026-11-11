# Runtime integration and first instructions

Build only for x86_64. On macOS, CMake selects x86_64 before enabling C,
C++ and assembly; Apple Silicon runs the resulting executables with Rosetta,
without a VM. Other platforms must already target x86_64.

`lib/runtime/cc/` contains relative symlinks to the original `runtime/` C,
header and assembly sources. Build them into a C11 static library without
`LAMA_ENV`, since the interpreter owns its stack. Keep the original sources
unchanged. `binop.hpp` binds directly to the original runtime functions using
assembly symbol names. CMake passes the platform-specific symbol prefix as a
public compile definition; no generated header is needed.
The original C11 tagging macro left-shifts signed negative integers; disable
only UBSan's `shift-base` instrumentation for the legacy C sources so sanitizer
builds can use that representation without editing the runtime.
Heap allocation and GC integration with the interpreter are future
work; the supported instructions allocate no runtime objects.

The interpreter keeps tagged 64-bit values in a downward-growing, typed stack.
State owns its storage and supplies checked `stack_push`, `stack_pop` and
`stack_top` helpers. `sp` and `bp` count stack elements; `ip` remains a byte offset
into the code. Stack failures leave the stack unchanged.
`runtime::box` and `runtime::unbox` wrap the original `BOX` and `UNBOX` macros in
C++ so the interpreter shares the runtime's representation. `CONST` sign-extends
its signed 32-bit operand and tags it. Every binop consumes the right operand
then the left operand, calls the corresponding `Ls__Infix_*` runtime function,
and pushes the tagged result. Advance `ip` by the instruction's wire size.
The `binop` helper takes its runtime function as a template argument. Use
`if constexpr` so only division and remainder include the zero-divisor check.
Report stack underflow/overflow and division or remainder by zero as errors.
Expose a read-only stack-top query for inspecting execution results.

`BEGIN` accepts only zero locals, otherwise reporting unsupported locals.
Ignore `n_args` for now, with `TODO(vityaman)` marking the missing handling.
`STRING` pushes `string_table.data() + string_offset` as a raw pointer in a
64-bit stack slot and advances `ip` by its wire size. It does not allocate or
tag the string. The table remains owned by the interpreter throughout execution.
Other instructions, including `END` and EOF, remain unsupported.

Validate all binops, operand order, negative constants, stack errors, and BEGIN
handling with unit tests. Compile the corpus examples `42.lama` and `43.lama`
(`42 + 1`) using LaMaC, check the stack results, and verify that the CLI reaches
the unsupported `END` diagnostic without crashing. Disassembly golden tests
must allow the execution diagnostic after producing their listing.
Compile `abacaba.lama` and verify that the stack holds a pointer to `"abacaba"`
when execution stops at `END`.
