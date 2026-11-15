# Std runtime interface

[`runtime/Std.i`](../../contrib/runtime/Std.i) exports 57 functions and one
global variable. It lists names, not types. The signatures below were checked
against [`runtime.c`](../../contrib/runtime/runtime.c),
[`runtime_common.h`](../../contrib/runtime/runtime_common.h), and
[`printf.S`](../../contrib/runtime/printf.S) on 2026-11-15. They describe the
64-bit C runtime; they do not imply that VM2 already supports every entry.

The [runtime bridge design](runtime-bridges.md) describes explicit C++ wrappers
and proposed runtime-call adapters. This inventory supplies native
signatures; argument-array and variadic adapter details remain separate design
questions.

## Type notation

LaMa is dynamically typed. The logical signatures describe the values accepted
and returned by the runtime, rather than static language declarations.

| Type | Meaning |
|:--|:--|
| `Value` | Any LaMa value: a tagged integer, managed pointer, or external pointer |
| `Int` | Tagged integer |
| `Bool` | `Int` containing zero or one |
| `String` | Pointer to a managed string's contents |
| `HeapValue` | Managed string, array, S-expression, or closure |
| `Array<T>` | Managed array with elements of type `T` |
| `List<T>` | A `cons` chain terminated by integer zero |
| `Sexp` | Managed S-expression |
| `File` | External `FILE *` returned by `fopen` |
| `Regex` | External `regex_t *` returned by `regexp` |
| `Unit` | No meaningful result; the native function returns `void` |
| `Never` | The function does not return |
| `A or B` | Either type, including `String or 0` for a string or tagged zero |

In native signatures, `aint` is `int64_t` on the supported 64-bit target.
An `aint` usually carries an already-tagged integer. A `void *` can also carry
tagged integer bits: its C type alone does not establish the logical value type.
`char *`, `FILE *`, and `regex_t *` carry actual pointers.

Native signatures preserve the implementation's types, including `void *`
returns for some integer results. Parameter names are descriptive. The source's
empty parameter lists, such as `Lread()`, are written as `(void)` to express
their actual zero-argument use. A native `void` return must not be read as a
returned LaMa word; any VM result for such a call needs an explicit convention.

## Functions with separate, fixed arguments

These functions receive each logical argument as a separate C argument.
Integer arguments remain tagged unless noted otherwise.

| Std name | Logical signature | Native C signature | Behavior |
|:--|:--|:--|:--|
| `getEnv` | `(name: String) -> String or 0` | `void *LgetEnv(char *name)` | Returns tagged zero if the environment variable is absent. |
| `system` | `(command: String) -> Int` | `aint Lsystem(char *command)` | Returns the tagged result of C `system`. |
| `stringInt` | `(text: String) -> Int` | `void *LstringInt(char *text)` | Returns integer bits through a pointer-typed result. |
| `makeArray` | `(length: Int) -> Array<Value>` | `void *LmakeArray(aint length)` | The current implementation fills elements with tagged zero. |
| `length` | `(value: HeapValue) -> Int` | `aint Llength(void *value)` | Returns the length encoded in the object header. |
| `hash` | `(value: Value) -> Int` | `aint Lhash(void *value)` | Returns a structural hash. |
| `fst` | `(value: HeapValue) -> Value` | `void *Lfst(void *value)` | Reads element zero through `Belem`; the element must exist. |
| `snd` | `(value: HeapValue) -> Value` | `void *Lsnd(void *value)` | Reads element one through `Belem`; the element must exist. |
| `hd` | `(list: List<Value>) -> Value` | `void *Lhd(void *list)` | Reads the head of a nonempty list. |
| `tl` | `(list: List<Value>) -> List<Value>` | `void *Ltl(void *list)` | Reads the tail of a nonempty proper list. |
| `readLine` | `() -> String or 0` | `void *LreadLine(void)` | Returns a string on a successful scan, otherwise tagged zero unless an input error terminates execution. |
| `matchSubString` | `(subject: String, pattern: String, position: Int) -> Bool` | `aint LmatchSubString(char *subject, char *pattern, aint position)` | Tests the substring at a zero-based position. |
| `regexp` | `(pattern: String) -> Regex` | `regex_t *Lregexp(char *pattern)` | Compiles a POSIX extended regular expression. |
| `regexpMatch` | `(pattern: Regex, subject: String, position: Int) -> Int` | `aint LregexpMatch(regex_t *pattern, char *subject, aint position)` | Returns the matched length at the given position, or tagged minus one. |
| `makeString` | `(length: Int) -> String` | `void *LmakeString(aint length)` | Allocates a managed string. |
| `fopen` | `(name: String, mode: String) -> File` | `FILE *Lfopen(char *name, char *mode)` | Returns an external file pointer; failure terminates execution. |
| `fclose` | `(file: File) -> Unit` | `void Lfclose(FILE *file)` | Closes a file. |
| `fread` | `(name: String) -> String` | `void *Lfread(char *name)` | Reads an entire file by name, opening and closing it internally. |
| `fwrite` | `(name: String, contents: String) -> Unit` | `void Lfwrite(char *name, char *contents)` | Writes a file by name, opening and closing it internally. |
| `fexists` | `(name: String) -> Bool` | `void *Lfexists(char *name)` | Returns tagged zero or one through a pointer-typed result; tests whether opening for reading succeeds. |
| `read` | `() -> Int` | `aint Lread(void)` | Prints a prompt and reads an integer. |
| `write` | `(number: Int) -> Int` | `aint Lwrite(aint number)` | Prints an integer and returns tagged zero. |
| `compare` | `(left: Value, right: Value) -> Int` | `aint Lcompare(void *left, void *right)` | Performs structural comparison. |
| `random` | `(limit: Int) -> Int` | `aint Lrandom(aint limit)` | Requires a positive limit; returns a number from zero to limit minus one. |
| `time` | `() -> Int` | `aint Ltime(void)` | Returns `CLOCK_MONOTONIC_RAW` time in microseconds; the implementation does not subtract program start time. |
| `kindOf` | `(value: Value) -> raw kind tag` | `aint LkindOf(void *value)` | Returns raw `1`, `3`, `5`, `7`, or `9`, without `BOX`. |
| `compareTags` | `(left: Sexp, right: Sexp) -> Int` | `aint LcompareTags(void *left, void *right)` | Returns the tagged difference between S-expression tags. |
| `flatCompare` | `(left: Value, right: Value) -> Int` | `aint LflatCompare(void *left, void *right)` | Performs shallow comparison; see the representation exception below. |
| `tagHash` | `(name: String) -> Int` | `aint LtagHash(char *name)` | Encodes an S-expression tag name. |
| `uppercase` | `(code: Int) -> Int` | `aint Luppercase(void *code)` | Takes tagged character-code bits through `void *`. |
| `lowercase` | `(code: Int) -> Int` | `aint Llowercase(void *code)` | Takes tagged character-code bits through `void *`. |

`kindOf` returns `STRING_TAG = 1`, `ARRAY_TAG = 3`, `SEXP_TAG = 5`,
`CLOSURE_TAG = 7`, or `UNBOXED_TAG = 9`. These are raw tags, even though they
are odd numbers and therefore resemble tagged integer words.

`flatCompare` returns a literal raw `-1` when its first argument is an integer
and the second is a pointer; its other return branches use `BOX`. The table
does not promise a consistently encoded comparison result in this implementation.

## Functions receiving an argument array

The logical arguments occupy consecutive mutable slots in source order. Each
slot preserves the LaMa word representation. The C function receives one
pointer to those slots, without a separate argument count. Allocating functions
can register these slots as GC roots and update them when objects move.

| Std name | Logical signature | Native C signature | Argument slots |
|:--|:--|:--|:--|
| `string` | `(value: Value) -> String` | `void *Lstring(aint *args)` | `args[0]`: value |
| `clone` | `(value: Value) -> Value` | `void *Lclone(aint *args)` | `args[0]`: value; an integer is returned unchanged |
| `stringcat` | `(strings: List<String>) -> String` | `void *Lstringcat(aint *args)` | `args[0]`: list; the implementation also accepts a string directly |
| `substring` | `(subject: String, position: Int, length: Int) -> String` | `void *Lsubstring(aint *args)` | `args[0]`: string pointer; `args[1]`: tagged position; `args[2]`: tagged length |
| `i__Infix_4343` (`++`) | `(left: String, right: String) -> String` | `void *Li__Infix_4343(aint *args)` | `args[0]`, `args[1]`: string pointers |
| `s__Infix_58` (`:`) | `(head: Value, tail: Value) -> Sexp` | `void *Ls__Infix_58(void **args)` | `args[0]`: head; `args[1]`: tail; constructs `cons(head, tail)` |

`substring` positions are zero-based. `:` produces a proper `List<T>` when its
tail is a proper list (including tagged zero for the empty list).

## Other operator functions

These are callable functions exported under encoded operator names. All receive
two separate C arguments. Integer inputs are passed as tagged bits through
`void *`; `aint` results are already tagged.

| Std name | Operator | Logical signature | Native C signature |
|:--|:--|:--|:--|
| `s__Infix_3333` | `!!` | `(left: Int, right: Int) -> Bool` | `aint Ls__Infix_3333(void *left, void *right)` |
| `s__Infix_3838` | `&&` | `(left: Int, right: Int) -> Bool` | `aint Ls__Infix_3838(void *left, void *right)` |
| `s__Infix_6161` | `==` | `(left: Value, right: Value) -> Bool` | `aint Ls__Infix_6161(void *left, void *right)` |
| `s__Infix_3361` | `!=` | `(left: Int, right: Int) -> Bool` | `aint Ls__Infix_3361(void *left, void *right)` |
| `s__Infix_6061` | `<=` | `(left: Int, right: Int) -> Bool` | `aint Ls__Infix_6061(void *left, void *right)` |
| `s__Infix_60` | `<` | `(left: Int, right: Int) -> Bool` | `aint Ls__Infix_60(void *left, void *right)` |
| `s__Infix_6261` | `>=` | `(left: Int, right: Int) -> Bool` | `aint Ls__Infix_6261(void *left, void *right)` |
| `s__Infix_62` | `>` | `(left: Int, right: Int) -> Bool` | `aint Ls__Infix_62(void *left, void *right)` |
| `s__Infix_43` | `+` | `(left: Int, right: Int) -> Int` | `aint Ls__Infix_43(void *left, void *right)` |
| `s__Infix_45` | `-` | `(left: Int, right: Int) -> Int`, also pointer subtraction | `aint Ls__Infix_45(void *left, void *right)` |
| `s__Infix_42` | `*` | `(left: Int, right: Int) -> Int` | `aint Ls__Infix_42(void *left, void *right)` |
| `s__Infix_47` | `/` | `(left: Int, right: Int) -> Int` | `aint Ls__Infix_47(void *left, void *right)` |
| `s__Infix_37` | `%` | `(left: Int, right: Int) -> Int` | `aint Ls__Infix_37(void *left, void *right)` |

`!!` computes logical OR. As ordinary function calls, both `!!` and `&&`
receive already-evaluated arguments. `==` compares word identity, whereas `!=`
in this implementation checks that both operands are integers. The pointer
branch of `-` subtracts its pointer operands and boxes the result.

The `L,"++",T,"+";` entry in `Std.i` declares the precedence of `++` at the
`+` level. It is not another function or global variable.

## Variadic functions

The following native signatures describe ordinary C targets. Arguments after
`...` use the native types required by the format string, rather than tagged
LaMa integers. String arguments remain pointers. The fixed condition argument
of `Lassert` remains a tagged LaMa integer.

| Std name | Logical signature | Native C target signature | Fixed C arguments |
|:--|:--|:--|:--|
| `assert` | `(condition: Int, format: String, ...values: Value) -> Unit` | `void Lassert(void *condition, char *format, ...)` | 2 |
| `sprintf` | `(format: String, ...values: Value) -> String` | `void *Bsprintf(char *format, ...)` | 1 |
| `printf` | `(format: String, ...values: Value) -> Unit` | `void Bprintf(char *format, ...)` | 1 |
| `fprintf` | `(file: File, format: String, ...values: Value) -> Unit` | `void Bfprintf(FILE *file, char *format, ...)` | 2 |
| `failure` | `(format: String, ...values: Value) -> Never` | `_Noreturn void failure(char *format, ...)` | 1 |

`assert` returns only when its condition is nonzero; otherwise it formats an
error and terminates execution. `failure` always formats an error and exits.
`Value` in the logical variadic signatures does not mean that every value can
be printed with every format specifier; each argument must match its specifier.

On x86-64, `Lprintf`, `Lfprintf`, `Lsprintf`, and `Lfailure` are assembly entry
points in `printf.S`. They use a compiler-specific convention, including an
argument count in `%r11`, and unbox integer arguments before jumping to
`Bprintf`, `Bfprintf`, `Bsprintf`, and `failure`, respectively. A C declaration
with `...` does not describe that additional register contract. The ordinary C
targets listed above are the relevant targets for an explicit VM call adapter.
`Lassert` is implemented directly in C and does not perform that unboxing.

The reference VM's [`ffi.c`](../../../virtual_machine/ffi.c) explicitly maps
the three formatted-output names to their `B` targets. Its metadata table does
not classify `assert` or `failure` as variadic; it is not a complete signature
inventory.

## Declared functions missing from the 64-bit runtime

| Std name | Logical signature | Native signature in `runtime32/runtime.c` | 64-bit status |
|:--|:--|:--|:--|
| `enableGC` | `() -> Unit` | `void LenableGC(void)` | No implementation found in `runtime/`. |
| `disableGC` | `() -> Unit` | `void LdisableGC(void)` | No implementation found in `runtime/`. |

These zero-argument signatures come from the
[`32-bit implementation`](../../../runtime32/runtime.c), not from definitions
in the 64-bit runtime. Their presence in `Std.i` alone does not supply linkable
64-bit functions.

## Global variable

| Std name | Logical type | Native declaration | Meaning |
|:--|:--|:--|:--|
| `sysargs` | `Array<String>` | `extern void *global_sysargs;` | Managed array containing the executable/program name and command-line arguments. |

The runtime defines storage as `void *global_sysargs`. Its
`void set_args(aint argc, char *argv[])` helper constructs the array and
registers the global slot as a GC root. This helper receives a raw C argument
count, not a tagged `Int`, and is not itself exported by `Std.i`.
