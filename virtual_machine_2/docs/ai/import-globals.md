# Imported public globals

Applied scope: add fixtures for imported global loads and stores. Do not change
the interpreter.

Extend `Sum1` with two public globals initialized to integer constants and add
matching `V` declarations to its interface. Add `Sum5`, importing `Sum1`, with
its own local global. Check both initial imported values, update both imported
globals using their loaded values, and check the results and the unchanged
local global. A failed check divides by zero so incorrect values produce a
nonzero exit rather than silently matching empty stdout. Do not use runtime
functions.

Validate the Sum fixtures with two sanitized trace runs. Inspect the compiled
`Sum5` listing for external global loads and stores, verify both VMs exit zero,
and confirm a temporary incorrect expected value fails. Run Python style checks.
