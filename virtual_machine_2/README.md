# Yet Another LaMa VM (by @vityaman this time)

## Principles

- **Fail fast**. Immediately exit with a simple message.

- **KISS**. Follow the `runtime` and `virtual_machine` style.
  This is an educational project, keep it as simple as possible.

- **Low LLM**. Write `.c` and `.h` files by hand.

- **MacOS**. Only MacOS is supported, but with Linux in mind.

## Getting Started

You need Nix with flakes.

Enter the shell.

```sh
nix develop
cd virtual_machine_2
```

Build the compiler and reference VM.

```sh
make lamac
make lamamv
```

Compile an example to the bytecode.

```sh
lamac -b -ds vm/examples/42.lama
```

It produces `42.bc` and `42.sm` in `$(pwd)`.

Run it via reference VM.

```sh
lamavm 42.bc
```
