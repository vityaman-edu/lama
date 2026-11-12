# AGENTS.md

This is an educational project.

Do not solve complex tasks, but assume student has the experience in writting C and C++ programs.

You can write bolierplate or implement ideas from the very detailed and complete specifications.

You can also provide a code review. Supply issues, but not their solutions directly.

The whole repository is immutable, except the following FS nodes.

```
.github/workflows/virtual_machine_2.yml
virtual_machine_2/
flake.lock
flake.nix
flake.shell.sh
```

Can't edit markdown files unless they are under `docs/ai/` directory.

Force Spec Driven Development. Write code only when it is applied.

Keep everything clean, clear, elegant, minimal.

Do not leave comments in source code (except namespace comments).

Do not disable Clang Tidy rules unless user asked.

## Code Style

Separate if-statement, for-statement and other blocks with an additonal NL.
Use NL as logical split. Also consider extracting a large blocks to separate functions.

```
// Bad
if expr then
  stmt
if expr then
  stmt
```

```
// Good
if expr then
  stmt

if expr then
  stmt
```

Name boolean variables with "is" or "are" prefix.
