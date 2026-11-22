# Runtime values and state

The runtime interface lives in
[`source/interpreter/runtime/`](../../source/interpreter/runtime/), in the
`lamavm::interpreter` namespace.

| Files | Responsibility |
|:--|:--|
| `box.hpp` / `box.cpp` | Tagged words and explicit integer/pointer conversion |
| `state.hpp` / `state.cpp` | Singleton stack, unit contexts, registers, and frames |
| `gc.hpp` / `gc.cpp` | C collector initialization and stack boundaries |

[`source/interpreter/std/`](../../source/interpreter/std/) contains the shared
bridge helpers and handwritten internal wrappers in `std.hpp` / `std.cpp`.
Its `std.py` catalogue generates the CALL wrappers and symbol table; clients
include `interpreter/std/std.g.hpp` from the CMake build directory.

The [Std inventory](std.md) lists native exports and their signatures. The
[runtime bridge design](runtime-bridges.md) describes the generic bridge and
generated and handwritten wrappers, together with the symbol registry and
proposed unconditional GC synchronization before FFI calls. Bytecode call
resolution and the common synchronization entry remain future changes.

## Word representation

`box` has one `int64_t` member. Its integer constructor applies the C runtime's
`BOX` encoding, and explicit integer conversion applies `UNBOX`. Pointer
construction and conversion preserve raw pointer bits. `unbox<T>` selects
integer or pointer conversion according to `T`.

`box{}` has a raw zero representation; `box(int64_t{0})` is tagged integer zero.
These representations have different meanings. Static assertions require
pointer-sized, standard-layout, trivially copyable boxes.

The arithmetic wrappers take and return boxes and preserve already-tagged bits
across the C ABI. Constructing a box from the returned raw integer would box
it a second time. `Bstring` takes a box containing a pointer to the string
argument slot and returns the allocated string pointer as a box.

## Deferred UBSan failures

The C runtime's [`BOX` macro](../../contrib/runtime/runtime_common.h) is
`((((aint)(x)) << 1) | 1)`, where `aint` is signed. Boxing a negative integer
triggers UBSan's `left shift of negative value` diagnostic in
[`runtime.c`](../../contrib/runtime/runtime.c). The C++ wrappers preserve the
runtime's tagged result; these failures occur inside the C runtime.

The following fixtures expose the defect. Paths are relative to `test/data`;
locations refer to `contrib/runtime/runtime.c` as checked on 2026-11-15.

| Fixtures | First failing operation | Location |
|:--|:--|:--|
| `examples/StdIO.lama`, `regression/test006.lama` | `Lread` boxes -7 and -6 respectively | 1277 |
| `examples/StdRegexp.lama` | `LregexpMatch` boxes the -1 mismatch result | 515 |
| `examples/StdStrings.lama` | `LstringInt` boxes -42 | 618 |
| `examples/StdValues.lama` | `Lcompare` boxes -1 | 641 |
| `regression/test004.lama` | Subtraction boxes -10 | 178 |
| `regression/test005.lama`, `regression/test007.lama`, `regression/test008.lama`, `regression/test012.lama` | Subtraction boxes -1 | 178 |
| `regression/test094.lama` | Subtraction boxes -2 | 178 |

Reproduce with `make lamavm2-san` followed by
`./scripts/test.sh san --runs=2`. Sanitizers halt at the first offending
operation, so later operations in each fixture remain unchecked. These runtime
defects are deferred and remain visible as sanitizer test failures; the
runner's comparison exceptions do not suppress them.

## Stack and frames

The singleton operand stack is a fixed vector of boxes and grows downward.
`sp` and `fp` are box pointers; `pc` points into word text; `up` selects the
current unit's strings, text, and global base. Globals occupy the high end of
the same stack, initialized to tagged zero before any unit body runs.

A frame occupies five words relative to `fp`.

| Slot | Value |
|:--|:--|
| `fp[0]` | Argument count |
| `fp[-1]` | Saved frame pointer |
| `fp[-2]` | Saved return instruction pointer |
| `fp[-3]` | Saved stack pointer |
| `fp[-4]` | Saved unit context |

Calls leave the evaluated arguments in place. Argument `i` is at
`fp[n_args - i]`. Locals start at `fp - 5` and grow downward. END preserves
the result, restores the four pointers, and pushes the result.

Fixed-arity instructions get their operand count from the opcode. CALL and
CALLC carry their argument count, and BEGIN and BEGIN_CLOSURE carry their
local count. The cold handlers use the decoded argument and capture indices to
address their slots. Warm specialization can store the resolved displacements.
The saved `pc` is the return address.

CALLC reads the closure operand when it dispatches to the callee. The closure
remains at `fp[n_args + 1]` for instructions that access capture values.
Unit startup pushes two synthetic arguments and saves a return address at the
shared EOF handler.

## GC boundary

`gc::init`, `gc::set_bot`, and `gc::set_top` wrap the existing C collector.
The stack bottom is registered before globals are pushed. `gc_call` publishes
the current top as `sp - 1` before invoking an allocating runtime operation.
Keep live boxed values in the collector-visible representation across that call.
