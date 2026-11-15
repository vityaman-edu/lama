# Std call example tests

The `test/data/examples/Std*.lama` cases exercise fixed-arity calls from the
runtime inventory in [std.md](std.md). Variadic calls are excluded. The `++`
operator is excluded because it is not registered in the current Std symbol table.

- `StdStrings`: string allocation and mutation, conversion, substring boundaries,
  concatenation, substring matching, and character case conversion.
- `StdValues`: array allocation and zero initialization, cloning, pair and list
  access, structural and shallow comparison, hashes, tags, and value kinds.
- `StdRegexp`: matches at an offset, mismatch, end of input, and an empty match.
- `StdIO`: integer input/output, line input, and end of input using a stdin fixture.
- `StdSystem`: environment lookup, command exit status, random bounds, monotonic
  time, and file calls using `/dev/null` without leaving files behind.

Checks print one on success and terminate on failure. Input/output checks print
results for comparison with the reference VM. Nondeterministic values are checked
through stable properties. The companion `.i` files select the existing
VM-only test convention because the source interpreter lacks some runtime calls.

Run with `./scripts/test.sh --filter='examples/Std*' --runs=2`.
