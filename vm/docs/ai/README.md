# VM knowledge

Durable behavior and maintenance guidance for `vm/`. Commands run from the
repository root; source links point to the owning implementation.

- [Bytecode and CLI](bytecode.md): decoding, listener lifetimes, dump conventions
  and command-line behavior.
- [Testing](testing.md): build/check entry points, fuzzing and Python linting.
- [Legacy golden tests](legacy-golden-tests.md): compiler comparison contract,
  normalization and expected compiler failures.

Follow [AGENTS.md](../../AGENTS.md) for contribution rules and
[the VM README](../../README.md) for initial setup.

Keep these notes organized by subject and update them when behavior changes.
Remove completed task checklists, run logs and discarded experiments after
capturing any lasting knowledge. Link to code for implementation details rather
than duplicating it; keep generated output out of this directory.
