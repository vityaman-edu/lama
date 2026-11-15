# Runtime bridges and symbol resolution

Status: generic bridge and Python wrapper generation applied on 2026-11-15.
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
All fixed-signature wrappers in [`std/`](../../source/interpreter/std/)
delegate to this bridge, replacing `unop`, `binop`, and handwritten native conversions.
The implementation lives in `std.hpp` under `lamavm::interpreter::detail`;
clients include the generated `interpreter/std/std.g.hpp`. Its return type is `box` for
non-void native results and `void` for native `void` results. The helper's
parameter pack uses `std::conditional_t<true, box, T>...` to accept one `box`
per native type.

Every converted argument and non-void result must have the same size as `box`
and be trivially copyable. These are compile-time requirements, consistent
with the [C++ bit-cast constraints](https://eel.is/c++draft/bit.cast). They do
not establish runtime pointer validity or semantic argument types.

## Generated declarations and definitions

`std/std.py` declares the 39 functions currently available through CALL as
flat Python lists: native return type, Std name, native name, then alternating
native parameter types and names, with an optional final `...`.
For example: `["void", "assert", "Lassert", "void*", "f", "char*", "s", "..."]`.
CMake runs this script to produce `std.g.hpp` and `std.g.cpp` under each build
directory's `generated/interpreter/std/`, and compiles the generated source.
The script is an explicit build dependency; generated files are not source files
maintained in the repository.

The generated header declares boxed and span wrappers and an extern
`std_symbols` table. The generated source declares native functions, defines
the wrappers, and defines the table once. Return types, fixed parameter counts,
and variadic flags are derived from the catalogue. Existing native signatures
and parameter names are preserved.

Each fixed native parameter becomes a `box` parameter. Every generated boxed
wrapper returns `box`: non-void native results preserve their representation,
and native `void` calls return `box{}` after invocation. Fixed-signature wrappers
delegate to `detail::bridge`, which still supports native `void` calls.

The 31 fixed-signature functions unavailable through CALL, including encoded
operators, `Li__Infix_4343`, and `Bmatch_failure`, are declared and defined
manually in `std.hpp` and its matching `std.cpp`. They have only boxed wrappers,
use the same helpers, and have neither span wrappers nor registry entries.

Applied variadic call specification: native declarations preserve their fixed
parameter types, names, and trailing `...`. Variadic span adapters call the
`extern "C"` targets directly through libffi; no boxed variadic wrappers are
generated. The unused manual boxed `failure` stub is removed. `failure` remains
outside the CALL registry.

The formatted-output wrappers use the ordinary C targets `Bsprintf`, `Bprintf`,
and `Bfprintf`, rather than the `L` assembly entry points. `LenableGC` and
`LdisableGC` have no implementation in the linked runtime and are not declared
or wrapped here.

## Uniform reversed-span overloads

Each generated boxed function also has a span overload returning `box`, matching
`std_fn_some_t`, which is `box (*)(std::span<box> rargs)`. The overloads
are enumerated in the same order as the boxed functions. They call those
existing boxed functions, preserving their native-parameter contract rather
than interpreting logical bytecode arguments.
The opcode unary/binary helper templates use explicit boxed function-pointer
types to select the original overloads.

The span views reversed stack arguments. Boxed parameter `i` receives
`rargs[rargs.size() - 1 - i]`. A shared invocation helper uses `LAMAVM_ASSERT`
to require exactly the fixed arity before expanding the call. The checks follow
the macro's existing build-mode behavior. The shared helper requires and returns
the boxed function's `box` result for fixed-signature calls.
`core/functional.hpp` provides `function_traits` for fixed and C-style variadic
function pointers, exposing `return_type`, `arity`, and `is_variadic`. Invocation
infers this information from the selected boxed function pointer. Span wrappers
call `detail::invoke_std(rargs, boxd::name)` without repeating boxed parameter types. Thin
overloads select boxed zero-argument or fixed signatures starting with `box`.
The variadic overload instead accepts the native function pointer. Count checks
and argument expansion remain shared for fixed-signature calls.

For variadic native functions, `invoke_std` forwards the entire reversed span
through libffi. It requires at least `sizeof...(T)` arguments and a total
count representable as `unsigned int`, prepares `ffi_prep_cif_var` with that
fixed count, and invokes the native function through `ffi_call`. Preparation
failures terminate with a diagnostic. These checks also apply in release builds.

Fixed arguments retain their `box` representation and use `ffi_type_pointer`,
matching the native fixed pointer parameters on the supported x86-64 ABI.
Trailing arguments use `box::is_ptr()`, defined as
`!UNBOXED(underlying)`, to distinguish pointers from tagged integers. Pointer
arguments preserve their addresses and use `ffi_type_pointer`; integer arguments
are unboxed through the integer conversion and use `ffi_type_sint64`. Mutable
integer and pointer storage and argument-address arrays live until the call
completes. Fixed tagged arguments, including `assert`'s condition, stay tagged.

The template accepts native pointer fixed parameters and a pointer or `void`
result. Pointer results preserve their returned bits in `box`. Native `void`
calls use `ffi_type_void` and return `box{}` without reading a native result.
Libffi setup lives in `std.cpp`.

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

The generated `std_symbols` table contains the 39 current CALL targets.
Keys use Std names, and entries select the corresponding `span`
wrapper, its fixed parameter count, and `vararg` for the four variadic
functions. Argument-array functions retain their current pointer-parameter
contract, including one fixed parameter for `substring`. Internal VM helpers,
the `sysargs` global, and the unavailable `enableGC` and `disableGC` functions
are excluded. Fixed-signature span-wrapper targets use `boxd::`; variadic
targets use `cc::` directly.
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

The implemented span overloads consume `std::span<box>` in reverse argument
order and return `box`. A native `void` CALL target produces `box{}` through
its generated boxed wrapper for fixed-signature calls, or directly through
libffi for variadic calls. The internal manual wrappers, including
`Bmatch_failure`, have no span adapters and retain their native
return convention through the low-level bridge.

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

`printf`, `fprintf`, `sprintf`, and `assert` use the direct span-to-native
libffi adapters described above. They target the ordinary C functions rather
than assembly entry points with compiler-specific calling conventions; see the
[native signature inventory](std.md#variadic-functions). `failure` remains
outside the CALL registry.

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
