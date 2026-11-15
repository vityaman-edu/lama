---
name: make-op
description: How to make a LaMa bytecode operation
---

# Make OP

Note. Paths are given relative to the repository root.

So... Human asked you to add another operation (let's name it OP) implementation in the lamavm2. Here is an algorithm, how to do it.

For example, an OP can be SEXP, ADD, etc.

Do it step-by-step. Stop after each step, waiting for user comments (OK means continue).

Wait using a prompt with option OK (second with own response will be added implicitly).

Also there is a YOLO mode. It means you should report, but not stopping after each step. Not YOLO by default.

## Step 1. See the OP specification

See the `virtual_machine/SPEC.md`.
Find and read the OP in the `SPEC.md`.
Report the markdown section link and summarise it in the chat.

For example:

> Found an OP in the SPEC.md: <link>.
> **BEGIN_CLOSURE**
> Enter a closure body and allocate local slots
> `BEGIN_CLOSURE:64 n_args:32 n_locals:32 n_captured:32`
>
> Pre  Stack: `... | arg_1 | ... | arg_n | closure`
> Post Stack: `... | arg_1 | ... | arg_n | closure | local_1 | ... | local_m`
> Effect:
> - "What it does description"
> - "What it does description"
>
> Example:
> Pre  Stack: ...
> Post Stack: ...
> Effect: ...

## Step 2. Undertand the OP usage

See the `src` (lamac, compiler).
Understand, when the instruction is emitted.
Report.

> The OP is emitted during xxx translation.
> It is used to xxx.
> For example,
> ```lama
> xxx
> ```
> translates to the following bytecode
> ```text
> xxx
> ```

## Step 3. See the OP implementation

See the `virtual_machine/ops.c`.
Find the `void op_<OP>(DECL_STATE)` function definition.
Report the code link.

> Found a void op_<OP>(DECL_STATE) in ops.c: <link>.

## Step 4. See the OP implementation dependencies

See the implementation. It can use some runtime functions.
For example, `Bstring`, `Bsexp`, `LtagHash`, etc.
Find each needed runtime function definition in `runtime` directory.
Report its signature and source location.

> There are runtime dependencies on xxx, yyy.
> <xxx-def-signature-from-runtime>: <link>
> <yyy-def-signature-from-runtime>: <link>

## Step 5. Make a runtime bridge

See the `virtual_machine_2/source/interpreter/runtime/std.hpp`.
For each runtime function make a declaration there.
Understand, whether it is a pure function, or uses allocator (gc).
Keep the same argument names.

```cpp
/* pure/alloc */ box <function>(args) // NOLINT
```

Then in `extern "C"` section at `std.cpp` add an entry
with the same signature as in definition from the `runtime/` directory.

```cpp
ret <function>(args) // NOLINT
```

Then add definition of box bridge.
Use `std::bit_cast` on each argument in a separate statement.
Then call the function and assign result to `r`.
Then `return std::bitcast<box>(r)`.

Report source references.

## Step 6. Make a cold handler

See the `virtual_machine_2/source/interpreter/cold/handler.cpp`.
Implement `void op_<OP>()` function there.
It is a good practice to add a `tracef` call.
Also add a handler mapping (switch-case).
Report source references.

## Step 7. Build

Ensure `make` is OK. Otherwise report failures and fix.
Continue without human approval.

## Step 8. Style

Ensure `make check` is OK. Otherwise report failures and fix.
Continue without human approval.

## Step 9. Test

In `virtual_machine_2/test/data/examples` add a single minimal `.lama` file that emits this `OP`.

Build: `make lamavm2-san-trace`.
Test: `./scripts/test.sh san --filter="examples/Op*" --runs=2`.

You can add `--show-stderr` to `test.sh`, so you can check traces.
