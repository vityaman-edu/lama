# Warm interpreter

Status: design decision from 2026-11-15; not implemented. The existing
[cold threaded interpreter](threaded-code.md) is the starting point.

## Cold and warm instructions

The cold interpreter executes instructions with their original integer operands
in the decoded opword representation. The intended cold path performs bounds
checks and resolves symbols at runtime. This is a design requirement, not a claim
that the current implementation already performs every required check.

After an instruction reaches an execution threshold, replace its cold handler
and operands with a warm handler and resolved operands. The threshold and the
counter storage are not decided yet.

Warming happens per instruction. A warm instruction may transfer control to
cold instructions. Keep instruction sizes unchanged so warming does not move
targets or saved return addresses.

## Decision: inline CALL operands

Keep CALL at two 64-bit opwords, including its handler:

```text
Cold: [cold_call handler] [target:i32 | n_args:i32]
Warm: [warm_call handler] [target_offset:i32 | n_args:i16 | unit_index:i16]
```

The diagrams describe field widths; bit positions and the C++ storage layout
remain to be specified.

All warm CALL operands remain inline. Use neither a pointer to a separate call
descriptor nor packed native pointers.

| Field | Width | Meaning |
|:--|--:|:--|
| `target_offset` | 32 bits | Opword offset from the target unit's text base |
| `n_args` | 16 bits | Number of arguments supplied by this CALL |
| `unit_index` | 16 bits | Index of the target unit in the runtime unit table |

The operand word occupies exactly 64 bits. Resolve a symbolic cold target into
its target unit and opword offset before warming. Store the target unit index
for local calls as well.

The warm CALL saves the caller's frame, including its current `up` and the
address immediately after the two-opword CALL. It selects the target unit as
`up`, derives `pc` from that unit's text base and `target_offset`, and dispatches
through the handler at `pc`. Returning restores the caller's `up` and `pc`.

The callee may still be cold: selecting its unit before dispatch gives its
unit-relative operands the correct context. Unit context is also needed by
operations using unit strings and globals, independently of their warmth.

This representation removes repeated symbol lookup from warm CALL while
retaining unit-table access and a base-plus-offset calculation. Its performance
has not been measured.

## Bounds and remaining choices

The proposed spelling uses signed `i32` and `i16` fields. A resolved target offset
must be nonnegative, identify an instruction boundary, and lie within the target
unit's text. Both `n_args` and `unit_index` must also be nonnegative.

With signed 16-bit fields, the maximum argument count is 32,767 and the maximum
unit index is 32,767, allowing 32,768 units. Using unsigned `u16` instead would
allow 65,535 arguments and 65,536 units; signedness is still to be finalized.

Validate bounds and representability before narrowing operands. Silent
truncation is not permitted. Whether an otherwise valid CALL that exceeds the
warm representation's limits stays cold or is rejected remains undecided.

Runtime unit indices must retain their meaning while warmed instructions exist.
The promotion mechanism must ensure that execution observes a matching handler
and operand layout. Counter storage, promotion timing, and publication details
remain to be specified before implementation.
