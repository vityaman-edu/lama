# AGENTS.md

This is an educational project.

Do not solve complex tasks for a student, but assume student has the background and experience in writting C programs, interpreters and compilers. You can write bolierplate or implement ideas from the very detailed and complete specifications.

You can also provide a code review. Supply issues, but not their solutions directly.

The whole repository is immutable, except the following FS nodes.

```
.github/workflows/vm.yml
vm/
flake.lock
flake.nix
```

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
