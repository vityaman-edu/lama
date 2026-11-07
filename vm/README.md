# Yet Another LaMa VM (by @vityaman this time)

This is an island of the personal LaMa VM source in the sea of LaMa ecosystem, the territory of tidiness.

## Getting Started

You need Nix with flakes.

Enter the shell.

```sh
nix develop
```

```bash
# Enter the devshell from the repository root. Keep using this shell below.
nix develop

# Build the compiler, 64-bit runtime, standard library, and bytecode VM.
./vm/scripts/build.sh

# Run the VM language and standard-library regression suites; failures exit nonzero.
./vm/scripts/test.sh

# Create a minimal LaMa program that prints 42.
printf 'write (42)\n' > Hello.lama

# Compile to Hello.bc in the current directory: -b selects bytecode,
# -runtime locates the runtime interface, and -I adds a module search path.
# Dune's -- separates its arguments from the compiler's arguments.
dune exec src/Driver.exe -- -runtime runtime -I _build/default/stdlib/x64 -b Hello.lama

# Run the bytecode on the VM; -I locates imported standard-library bytecode.
dune exec virtual_machine/lama.exe -- -I _build/default/stdlib/x64 Hello.bc

# Print bytecode metadata and instructions without executing the program.
dune exec virtual_machine/lama.exe -- --disassemble Hello.bc

# Run the same source with the reference source interpreter (-i).
# It reads stdin up front; /dev/null supplies EOF for this input-free example.
dune exec src/Driver.exe -- -runtime runtime -i Hello.lama < /dev/null

# Run the reference stack-machine interpreter (-s) for comparison.
dune exec src/Driver.exe -- -runtime runtime -s Hello.lama < /dev/null

# Display the VM's command-line options.
dune exec virtual_machine/lama.exe -- --help

# Leave the devshell when finished.
exit
```
