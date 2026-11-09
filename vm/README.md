# Yet Another LaMa VM (by @vityaman this time)

This is an island of the personal LaMa VM source in the sea of LaMa ecosystem, the territory of tidiness.

## Getting Started

You need Nix with flakes.

Enter the shell.

```sh
nix develop
```

Build the driver.

```bash
./vm/scripts/build.sh
```

Test the interpreter.

```bash
./vm/scripts/test.sh
```

Compile an example to the bytecode.

```bash
lamac -b -ds vm/examples/42.lama
```

It produces `42.bc` and `42.sm` in `$(pwd)`.
