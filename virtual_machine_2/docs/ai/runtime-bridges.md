# Runtime bridges and symbol resolution

Status: generic bridge and explicit wrapper enumeration applied on 2026-11-15.
Common GC call entry and symbol resolution remain design decisions; those
sections do not authorize source implementation. The [Std inventory](std.md)
describes the existing native functions; [runtime values and state](runtime.md)
describes the current wrappers and GC boundary.

## Generic bridge

One generic function named `bridge` connects boxed C++ calls
to native runtime functions. Its template inputs are the native return type,
the compile-time-known native function pointer, and the native argument types.
The template argument order is `bridge<R, Function, Args...>`.

For each fixed argument, the bridge receives a `box` and uses `std::bit_cast`
to obtain the corresponding native type. It calls the function through its
actual declared type. A non-void result is bit-cast back into `box`. A `void`
result requires a separate compile-time branch with no result bit-cast.

The conversions preserve representations. They do not apply `BOX`, `UNBOX`,
the integer constructor of `box`, or `unbox<T>`. Runtime arguments and results
that encode integers are already tagged. Pointer bits also remain unchanged.
All fixed-signature wrappers in [`std.cpp`](../../source/interpreter/runtime/std.cpp)
delegate to this bridge, replacing `unop`, `binop`, and handwritten native conversions.
The implementation remains private to that translation unit; public wrapper
declarations in `std.hpp` retain their signatures. Its return type is `box` for
non-void native results and `void` for native `void` results. The parameter pack
uses `std::conditional_t<true, box, T>...` to accept one `box` per native type.

Every converted argument and non-void result must have the same size as `box`
and be trivially copyable. These are compile-time requirements, consistent
with the [C++ bit-cast constraints](https://eel.is/c++draft/bit.cast). They do
not establish runtime pointer validity or semantic argument types.

## Explicit declarations and definitions

The boxed declarations in `std.hpp`, native declarations in `std.cpp`, and
boxed definitions in `std.cpp` explicitly enumerate the same 71 functions in
runtime definition order. They cover implemented Std exports and the existing
VM helpers. Every parameter name matches its native definition, including
`p` and `q` for binary operators, `args` for argument-slot pointers, and `s`
for `LtagHash`. Native signatures preserve the runtime types, including
`char*` rather than `const char*` for `LtagHash`.

Each fixed native parameter becomes a `box` parameter. Non-void native results
become `box` results; native `void` results remain `void`. Fixed-signature
wrappers delegate to `bridge`. Declarations remain in the header and definitions
in the matching `.cpp`; wrapper generation macros are not used.
Functions flagged by Clang Tidy carry `NOLINT`, as requested, preserving native
identifiers and the explicit C-style variadic stubs.

Variadic native declarations preserve their fixed parameter types, names, and
trailing `...`. Boxed declarations and definitions replace each fixed type with
`box` and retain `...`. The five variadic boxed definitions (`failure`,
`Lassert`, `Bsprintf`, `Bprintf`, and `Bfprintf`) call `fatalf` with an unsupported
variadic call diagnostic. They do not forward arguments or call native code.

The formatted-output wrappers use the ordinary C targets `Bsprintf`, `Bprintf`,
and `Bfprintf`, rather than the `L` assembly entry points. `failure` uses the
ordinary C target of the same name. `LenableGC` and `LdisableGC` have no
implementation in the linked runtime and are not declared or wrapped here.

## GC synchronization

Synchronize GC state before every FFI call, regardless of the callee. FFI here
means the VM-to-native-runtime boundary; it does not imply using libffi.

The common call entry owns synchronization before invoking its adapter.
`bridge` handles representation conversion and invocation, without choosing
whether a function needs GC synchronization. Live arguments remain rooted on
the VM stack across calls that may collect. Publishing the current stack
boundary does not itself request a collection.

The current `gc_call` publishes `sp - 1` through `gc::set_top`. The new common
boundary must preserve that stack-scanning contract. Existing explicit
`gc_call` sites remain the current implementation until the design is applied.

## Symbol registry and span adapters

Resolve a runtime function name through an explicit name-to-call registry.
Registry entries refer to adapters with a common callable interface.
Each adapter's target function and native signature are known at compile time.
Fixed-signature invocation therefore requires neither libffi nor `dlsym`.

For ordinary fixed-signature functions, an adapter receives a span of logical
LaMa arguments, checks that its length equals the expected arity, and expands
the elements into a call of the known boxed wrapper. The bridge then invokes
the native function through its declared signature. No native function pointer
is cast into the common adapter's function-pointer type.

```text
CALL name, n_args
    -> resolve runtime name to a registry entry
    -> check logical argument count
    -> synchronize GC state
    -> invoke the uniform span adapter
    -> invoke the boxed wrapper and generic bridge
    -> call the statically linked native function
```

This describes runtime calls only. Integration with bytecode-defined functions,
lookup precedence, resolution timing, and caching is not yet specified. The
registry's container and exact entry representation also remain open. Runtime
names must map to their intended native targets: for example, `compare` denotes
`Lcompare`; bytecode names and native symbol names are distinct.

The original proposed input is `std::span<const box>` containing `n_args`
elements. It suffices for reading separate fixed arguments, but the complete
adapter interface remains open because argument-array calls need writable
storage. The uniform return convention also remains open, particularly the VM
value produced after a returning native `void` function. The low-level bridge
must not invent that result by reading a nonexistent native return value.

## Argument-array functions

Native parameter types alone do not determine the bytecode argument contract.
For example:

| Function | Logical arguments | Native arguments |
|:--|:--|:--|
| `compare` | Two values | Two separate values |
| `substring` | String, position, length | One pointer to three argument slots |
| `clone` | One value | One pointer to its argument slot |
| `Barray`, an internal helper | Array elements supplied by its opcode | Argument-storage pointer and tagged element count |

The boxed wrapper for `Lsubstring(aint *)` takes one box containing a pointer.
Its bytecode adapter must instead accept three logical arguments and arrange
the required argument storage. Expanding three span elements into that
one-parameter wrapper is not the right call. Internal helpers such as `Barray`
also need to remain distinct from the functions exported by `Std.i`.

The registry must describe or select the appropriate logical calling shape.
How these special adapters are declared and generated remains undecided; the
native signatures do not encode their logical arity.

Argument-array functions can register slots as GC roots and rewrite them after
objects move. Their storage must therefore support those writes. A const span
alone does not express this contract. Also, equal sizes and pointer bit-casts
do not establish that a `box` array can legally be accessed as an `aint` or
`void *` array. Storage lifetime, aliasing, mutability, and propagation of GC
updates must be specified before implementing these adapters.

## Variadic functions

The explicit variadic wrappers currently fail through `fatalf`, as described
above. Native variadic forwarding and executable span adapters remain deferred.
A fixed-signature bridge cannot infer a runtime argument pack.

`printf`, `fprintf`, `sprintf`, `assert`, and `failure` require this later
decision. On x86-64, their assembly entry points also have additional calling
requirements; see the [native signature inventory](std.md#variadic-functions).

## Runtime validation boundary

Correct declarations, adapters, and exact argument counts establish
the native call shape. They do not prove that a LaMa word is a valid string,
array, external handle, or pointer. Bit-casting supplies no such validation.
The runtime's current checks do not make arbitrary bytecode memory-safe.

Pointer/value validation and any sandbox policy remain separate design work.
The explicit registry limits which native targets are exposed, but functions
such as `system` and file I/O still perform their documented process-level
operations when exposed. `sysargs` is a global variable and needs a separate
global-symbol path; it is not a callable bridge.
