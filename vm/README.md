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

# Build the compiler and its source and stack-machine interpreters.
./vm/scripts/build.sh

# Run the source-interpreter language regressions; failures exit nonzero.
./vm/scripts/test.sh

# Create a minimal LaMa program that prints 42.
mkdir -p vm/scratch
printf 'write (42)\n' > vm/scratch/Hello.lama

# Run the reference source interpreter (-i).
# It reads stdin up front; /dev/null supplies EOF for this input-free example.
lamac -i vm/scratch/Hello.lama < /dev/null

# Run the reference stack-machine interpreter (-s) for comparison.
lamac -s vm/scratch/Hello.lama < /dev/null

# Generate bytecode for the custom VM in the scratch directory.
(cd vm/scratch && lamac -b Hello.lama)

# Leave the devshell when finished.
exit
```

The default build omits the native runtime, compiled standard library, and C VM.
The reference interpreters support core language operations and a limited set of
builtins; they do not provide the full C runtime or standard-library environment.
The `lamavm0` shell function still points to the optional C VM, which this build
does not produce.
