# Yet Another LaMa VM (by @vityaman this time)

## Principles

- **Fail fast**.
  Immediately exit with a simple message.

- **Compile-time paramters**.
  Parameterization is implemented via macro definitions.

- **Single Thread**.
  Only this is supported by the LaMa runtime.

## Features

- **Weak Validation**.
  Comparing to the reference `virtual_machine`, the `virtual_machine_2`
  does not validate stack depth and bytecode layout.

- **Lazy Name Resolution**.
  Name resolution is done in runtime on first intruction execution.

- **Uniform Bytecode**.
  The packed bytecode is transformed into corresponding 64-bit words
  without any decoding and pointers resolution.
  The first word is always a function pointer -- a handler.
  Arguments are passed as properly aligned following bytes.

- **No Units Merge**.
  Units are loaded into separate memory areas and a special argument
  `up` (unit pointer) is used to resolve offsets to strings and code labels.
  This is done to support easy dynamic unit loading and save time on relocation.

- **Runtime Specialization (Code Patching)**.
  VM starts with a cold interpreter with some hardening.
  Cold interpreter lookups strings, external references.
  After a cold interpreter have executed the instruction kernel,
  it specializes current instruction.
  It is done by replacing the handler and arguments to be more specific.
  For example,
  `call_local` does not require `up`,
  `call_std` does not allocate a stack frame,
  `begin_no_locals` just skips `n_locals` argument,
  string and code offsets are replaced with a direct pointers.

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
