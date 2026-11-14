# Dependency closure and linking

Status: design discussion for
[`source/bc/closure.cpp`](../../source/bc/closure.cpp). The proposed opword
pipeline is not implemented.

## Goal

Load `Main.bc` and all its bytecode dependencies and produce one self-contained
program. Later processing should not need to load imports or resolve bytecode
module dependencies.

Combining modules and resolving their references is linking. This does not
require replacing individual call sites with copies of function bodies.
Host/runtime calls still require resolved native targets and a calling
convention; dependency closure does not turn them into generated native code.

## Representation boundaries

- Keep the file representation compact and independent of runtime addresses.
- Read header and metadata fields into ordinary values as needed.
- Build a symbol-name index where repeated lookup occurs during linking.
- Decode instructions into the proposed [opword representation](opword.md).
- Make the resulting program own all storage its resolved pointers reference.

An `unordered_map` is useful for repeated name resolution, but is not required
for metadata that is only traversed. Once references are resolved, execution
need not perform symbol-name lookup. A temporary linker index can be discarded
unless exports, diagnostics, or another concrete feature still needs it.

The program can retain the original string buffers or own copied string storage.
One owning program does not require every string to occupy one allocation.

## Independent decoding

Decoding each module independently, then merging decoded word arrays, is a
reasonable construction pipeline. The reference VM does this in `decode` and
`link_program` in [converter.c](../../../virtual_machine/converter.c).

Two conceptual states were discussed:

| State | Reference representation |
|:--|:--|
| Relocatable module | Local indices or offsets and unresolved symbols |
| Executable program | Resolved code, string, and global references |

Symbolic relocation records can represent unresolved targets. After all targets
are resolved to pointers, moving unchanged code can instead use instruction-aware
pointer enumeration and region rebasing. Separate records are not required just
to identify pointer operands when the decoder already knows them. See
[relocation](relocation.md).

Disk byte offsets and decoded word positions are different coordinates. Branches,
calls, closure targets, and exported function addresses need translation to the
corresponding decoded instruction boundaries.

## Packed intermediate output

The original idea was to concatenate packed sections and update their offsets.
That remains useful if a self-contained `.bc` file is an intended output.

For execution alone, constructing a packed merged unit and then decoding it is
an optional intermediate step. The proposed alternative is to link the decoded
modules directly into the execution representation.

## Open decisions

- Is the output a serializable linked bytecode unit, an executable program,
  or are both required?
- When are external symbols resolved relative to module layout and merging?
- How are module initialization order and exported entry points represented?
- Which storage remains owned by the final program?
- Is executable storage initially fixed, with runtime movement deferred?

The separation between module construction and execution leaves room for later
optimization or JIT work without requiring live-code relocation now.
