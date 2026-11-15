# Runtime bridges and symbol resolution

Status: generic bridge and explicit wrapper enumeration applied on 2026-11-15.
Common GC call entry and symbol resolution remain design decisions; those
sections do not authorize source implementation. The [Std inventory](std.md)
describes the existing native functions; [runtime values and state](runtime.md)
describes the current wrappers and GC boundary.

## Generic bridge

One generic function named `bridge` connects boxed C++ calls
to native runtime functions. Its only explicit template argument is the
compile-time-known native function pointer: `bridge<Function>(args...)`.
A typed helper deduces the native return type and parameter types from that
pointer, so boxed wrappers do not repeat the native signature.

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
non-void native results and `void` for native `void` results. The helper's
parameter pack uses `std::conditional_t<true, box, T>...` to accept one `box`
per native type.

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

## Uniform reversed-span overloads

Each boxed function also has a span overload. Value-returning overloads match
`std_fn_t`, which is `box (*)(std::span<const box> rargs)`; void overloads match
`std_fn_void_t`, which is `void (*)(std::span<const box> rargs)`. The overloads
are enumerated in the same order as the boxed functions. They call those
existing boxed functions, preserving their native-parameter contract rather
than interpreting logical bytecode arguments.
The opcode unary/binary helper templates use explicit boxed function-pointer
types to select the original overloads.

The span views reversed stack arguments. Boxed parameter `i` receives
`rargs[rargs.size() - 1 - i]`. A shared invocation helper uses `LAMAVM_ASSERT`
to require exactly the fixed arity before expanding the call. The checks follow
the macro's existing build-mode behavior. The shared helper preserves the boxed
function's `box` or `void` return type, with a compile-time branch for the void
call.
`core/functional.hpp` provides `function_traits` for fixed and C-style variadic
function pointers, exposing `return_type`, `arity`, and `is_variadic`. Invocation
infers this information from the selected boxed function pointer. Span wrappers
call `invoke_std(rargs, name)` without repeating boxed parameter types. Thin
overloads select the boxed function from its overload set: zero-argument,
fixed signatures starting with `box`, and variadic signatures starting with
`box`. Count checks and argument expansion remain shared.

For variadic boxed functions, the helper asserts at least the fixed argument
count and passes the fixed arguments in the same reversed order. The boxed
function then reports its existing unsupported-call diagnostic. Variadic
forwarding remains unimplemented.

Argument-storage functions retain their boxed pointer parameters: for example,
the `Lsubstring` span overload takes one box containing an argument-slot pointer,
and `Barray` takes that pointer and its boxed count. These overloads do not
prepare argument arrays, synchronize GC state, or implement a symbol registry.

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

The `std_symbols` table is applied for the 55 implemented functions exported by
`Std.i`. Keys use Std names, and entries select the corresponding `span`
wrapper, its fixed boxed parameter count, and `vararg` for the five variadic
functions. Argument-array functions retain their current pointer-parameter
contract, including one fixed parameter for `substring`. Internal VM helpers,
the `sysargs` global, and the unavailable `enableGC` and `disableGC` functions
are excluded. Direct boxed calls and span-wrapper targets use `boxd::`.
This table does not apply bytecode call resolution or logical argument-array
adaptation; those remain design decisions below.

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

The implemented `std_fn_t` and `std_fn_void_t` overloads consume
`std::span<const box>` in reverse argument order and preserve `box` or `void`
returns. Bytecode-level argument-array adapters still need writable storage.
Any VM stack value produced after a returning native `void` function remains a
separate decision; the low-level bridge does not invent a native return value.

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
