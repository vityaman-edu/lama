# Yet Another LaMa VM (by @vityaman this time)

## Principles

- **Fail fast**. Immediately exit with a simple message.

- **KISS**. Follow the `runtime` and `virtual_machine` style.
  This is an educational project, keep it as simple as possible.

- **Low LLM**. Write it by hand.

- **MacOS**. Only MacOS is supported, but with Linux in mind.

- **Compile-time paramters**. Parameterization is implemented
  via macro definitions, to keep it simple.

- **Single Thread**. Only this is supported by the LaMa runtime.

## Getting Started

You need Nix with flakes.

Enter the shell.

```sh
nix develop
cd virtual_machine_2
```

Build the compiler and VMs.

```sh
make lamac
make lamamv
make lamamv2
```

Compile an example to the bytecode.

```sh
lamac -b -ds vm/examples/42.lama
```

It produces `42.bc` and `42.sm` in `$(pwd)`.

Run it via VMs.

```sh
lamavm 42.bc
```

Run differential tests.

```sh
make test
```

## Roadmap

### Procedures

The goal is to correctly run the `42.lama`.

- BEGIN
- CONST
- END
- EOF

### Simple Instructions

The goal is to support more interesting expressions.

- BINOP
- STRING
- JMP
- CJMP
- DROP
- DUP
- SWAP

### Internal Call

Call to the current bytecode file.

- CALL
- LD_LOC
- LD_ARG
- ST_LOC
- ST_ARG

### Globals

Support global variables.

- LD_GLO
- ST_GLO

### External Call

Call to the external bytecode file.

### SExpr

- SEXP
- TAG
- ELEM

### Array

- BARRAY
- ARRAY

### Clojure

- BEGIN_CLOJURE
- CLOSURE
- CALLC
- ST_CLO
- LD_CLO

### Pattern Matching

- FAIL
- PATT_STR_CMP
- PATT_STRING
- PATT_ARRAY
- PATT_SEXP
- PATT_BOXED
- PATT_UNBOXED
- PATT_CLOSURE

### Other

- LINE

## Redesign

### Input

The input of `lamavm2` is an only `.bc` file.

```sh
./lamavm2 Main.bc
./lamavm2 ./Main.bc
./lamavm2 path/to/Main.bc
```

The main unit can reference external symbols (globals or functions).
Only public symbols of imported units can be referenced.
Imported units are found relative to the main unit directory.

### Parsing

The parsing transforms **bytes** into the **unit**.

### Unit Closure

The unit closure recursively resolves imported modules.
The closure is a single module.
Module contents are written in a some topological order.
String Tables are merged, but not interned.
Global Arrays are appended.
Imports are gone.
Public symbols are not merged, only top-level are leaved.
Relocation is done.

### Analysis

TODO

## T O D O

- Span that has end only at DEBUG and checks range bounds
- Strong Alias Box for stack content and Std functions args
