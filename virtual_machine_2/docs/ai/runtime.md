# Runtime values and state

The runtime interface lives in
[`source/interpreter/runtime/`](../../source/interpreter/runtime/), in the
`lamavm::interpreter` namespace.

| Files | Responsibility |
|:--|:--|
| `box.hpp` / `box.cpp` | Tagged words and explicit integer/pointer conversion |
| `state.hpp` / `state.cpp` | Singleton stack, unit contexts, registers, and frames |
| `gc.hpp` / `gc.cpp` | C collector initialization and stack boundaries |
| `std.hpp` / `std.cpp` | Explicit C++ wrappers for implemented Std exports and VM helpers |

The [Std inventory](std.md) lists native exports and their signatures. The
[runtime bridge design](runtime-bridges.md) describes the generic bridge and
explicit wrappers, together with the proposed symbol registry and unconditional
GC synchronization before FFI calls. The registry and common synchronization
entry remain future changes.

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

The C runtime's [`BOX` macro](../../contrib/runtime/runtime_common.h) uses a
signed left shift. Negative arithmetic results can therefore trigger UBSan in
[`runtime.c`](../../contrib/runtime/runtime.c); the C++ wrappers preserve that
runtime behavior.

## Stack and frames

The singleton operand stack is a fixed vector of boxes and grows downward.
`sp` and `fp` are box pointers; `pc` points into word text; `up` selects the
current unit's strings, text, and global base. Globals occupy the high end of
the same stack, initialized to tagged zero before any unit body runs.

A frame occupies five words. Relative to `fp`, it stores:

| Slot | Value |
|:--|:--|
| `fp[0]` | Tagged argument count |
| `fp[-1]` | Saved frame pointer |
| `fp[-2]` | Saved return instruction pointer |
| `fp[-3]` | Saved stack pointer |
| `fp[-4]` | Saved unit context |

Arguments are above the frame; locals are below it. END preserves the result,
restores the four saved pointers, and pushes the result on the caller's stack.
The saved unit context is necessary for returning from imported function calls.

## GC boundary

`gc::init`, `gc::set_bot`, and `gc::set_top` wrap the existing C collector.
The stack bottom is registered before globals are pushed. `gc_call` publishes
the current top as `sp - 1` before invoking an allocating runtime operation.
Keep live boxed values in the collector-visible representation across that call.
