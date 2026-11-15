# Linking decoded modules

Status: future design. The current [loader](units.md) builds a dependency closure
and public-symbol index, retains separate buffers, and resolves external CALL
and global operands during cold execution.

## Representation boundaries

Keep serialized bytecode compact and independent of runtime addresses. Decode
each module independently, then establish final storage and resolve references.
The reference VM's [converter](../../../virtual_machine/converter.c) illustrates
separate decoding and linking phases.

| State | Reference representation |
|:--|:--|
| Relocatable module | Local indices/offsets and unresolved symbols |
| Executable program | References resolved for its chosen execution layout |

Disk byte offsets and decoded word positions are different coordinates.
Branches, calls, closure targets, and exported functions must refer to decoded
instruction boundaries. The current parser already converts local targets.

A program must own all storage its references use. One owning program need not
put every unit or string into a single allocation. Stable separate allocations
can avoid relocation while still supporting optimization or generated handlers.

## Resolution and movement

A symbol-name index supports repeated resolution. It can be discarded only when
execution and other required features no longer need it; cold execution currently
does. The [warm CALL decision](warm-interpreter.md) uses unit indices and word
offsets rather than resolved native pointers.

Symbol resolution establishes target identity. [Pointer relocation](relocation.md)
translates already-known addresses when storage moves; these are separate tasks.
Instruction-aware traversal can identify pointer operands if each handler's
layout is known. Unresolved symbolic targets may still require fixup records.

## Remaining design choices

- Whether a serializable linked `.bc` file is needed in addition to execution.
- Whether to merge unit storage or retain stable buffers and unit coordinates.
- When to resolve each external reference relative to warming and final layout.
- Which exports, symbol metadata, and initialization entry points must survive.

Concatenating packed sections is useful for producing a linked file, but is an
optional intermediate step for execution. Linking resolves module references;
it does not inline function bodies or turn C runtime calls into generated code.
