# Yet Another LaMa VM (by @vityaman this time)

## Getting Started

You need Nix with flakes.

Enter the shell.

```sh
nix develop
```

Compile an example to the bytecode.

```bash
lamac -b -ds vm/examples/42.lama
```

It produces `42.bc` and `42.sm` in `$(pwd)`.
