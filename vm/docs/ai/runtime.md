# Runtime integration and first instructions

Build only for x86_64. On macOS, CMake selects x86_64 before enabling C,
C++ and assembly; Apple Silicon runs the resulting executables with Rosetta,
without a VM. Other platforms must already target x86_64.

`lib/runtime/cc/` contains relative symlinks to the original `runtime/` C,
header and assembly sources. Build them into a C11 static library without
`LAMA_ENV`, since the interpreter owns its stack. Keep the original sources
unchanged. `binop.hpp` and `sexp.hpp` bind directly to the original runtime functions using
assembly symbol names. CMake passes the platform-specific symbol prefix as a
public compile definition; no generated header is needed.
The original C11 tagging macro left-shifts signed negative integers; disable
only UBSan's `shift-base` instrumentation for the legacy C sources so sanitizer
builds can use that representation without editing the runtime.
`SEXP` allocates runtime objects using the original moving collector.

The interpreter keeps tagged 64-bit values in a downward-growing, typed stack.
`interpreter::stack` has only static methods and manages a fixed singleton
stack with checked `push`, `pop`, `drop`, and `top` operations. It owns the stack
pointer; state retains only the base pointer and instruction pointer. Stack
positions count elements; `ip` remains a byte offset into the code. Stack
failures leave the stack unchanged.
`runtime::box` and `runtime::unbox` wrap the original `BOX` and `UNBOX` macros in
C++ so the interpreter shares the runtime's representation. `CONST` sign-extends
its signed 32-bit operand and tags it. Every binop consumes the right operand
then the left operand, calls the corresponding `Ls__Infix_*` runtime function,
and pushes the tagged result. Advance `ip` by the instruction's wire size.
The `binop` helper takes its runtime function as a template argument. Use
`if constexpr` so only division and remainder include the zero-divisor check.
Report stack underflow/overflow and division or remainder by zero as errors.
Use `stack::top()` to inspect execution results without modifying the stack.

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


## S-expression runtime interface

An s-expression is a constructor tag and an ordered sequence of runtime values,
for example `Pair(10, 20)` or the nullary constructor `Nil`.
On x86_64 without `DEBUG_VERSION`, its allocation contains 64-bit words:
`(n_fields << 3) | 5`, GC forwarding metadata, the unboxed constructor-tag
hash, then the fields in source order. The value pointer addresses the tag word,
16 bytes after the allocation base; the first field is 8 bytes after that pointer.
Integers in fields retain their low-bit tagging; nested s-expressions are pointers.

`runtime::tag_hash` binds to `LtagHash`, which returns a boxed tag encoding.
`runtime::sexp` binds to `Bsexp` and takes a mutable array
`[field0, ..., fieldN-1, boxed_tag_hash]` and `box(N + 1)`.
`Bsexp` registers field slots as extra GC roots, allocates the object, copies
fields in array order, stores the unboxed tag hash, unregisters the roots,
and returns a pointer to the tag word. A collection can update the argument
array's field pointers. The runtime heap must be initialized and all other
live VM values must be visible to the collector before calling this function.

## SEXP execution and heap lifetime

`SEXP tag_offset, n_fields` rejects negative field counts, insufficient stack
operands, invalid tag offsets, and stack overflow for a nullary constructor.
These errors preserve the stack and instruction pointer. Resolve the constructor
name at `tag_offset` in the owned string table. Copy the top `n_fields` values
into a temporary array in reverse stack order, restoring their source order;
append `runtime::tag_hash(name)`. Call `Bsexp` with this array and the boxed
array size. On success, consume the fields, push the returned pointer as a
64-bit value, and advance `ip` by the nine-byte instruction size.

`interpreter::stack` owns one static array of 1 MiB of value slots plus a
sentinel word preceding them. Its public methods are static; construction is
disabled. `state` acquires this singleton during construction and releases it
during destruction. An atomic flag rejects a second live state with
`an interpreter is already active`, including simultaneous construction from
another thread. A failed acquisition must not reset the live stack or heap.
Releasing the state resets the stack and permits the next interpreter to start
empty. State is noncopyable and nonmovable and owns no stack storage or span.

Initialize the C runtime heap when acquiring the stack and shut it down before
releasing ownership. Set the collector's bottom bound to the end of the static
array. After initialization and every successful stack mutation, its top bound
points to the word immediately before the active stack range. This word is the
sentinel when the stack is full; the interval is valid and empty when no values
are pushed. The collector scans and updates the singleton stack directly.
`Bsexp` separately roots its temporary argument fields. Remove the interpreter
registry, per-allocation stack-root registration, and shared heap owner class.
The original C runtime sources remain unchanged.

Validate nullary constructors, source field order, signed integer fields,
nested objects, instruction advancement, stack errors, and invalid operands.
Force collections that relocate both argument-array pointers and unrelated
live stack pointers. Reject nested and concurrent interpreter construction
without disturbing the owner, and verify stack and heap reinitialization after
that interpreter has been destroyed.
