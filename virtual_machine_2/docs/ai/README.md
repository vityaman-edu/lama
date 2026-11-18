# VM knowledge

This directory is for durable project knowledge only. Do not store temporary
notes, task plans, implementation checklists, review transcripts, or test-run
reports here. Consolidate lasting conclusions into the relevant topic and remove
superseded documents. Keep the index complete when adding or removing a topic.

Implementation descriptions were checked against the source on 2026-11-15.
Design decisions below remain separate from implemented behavior and do not
authorize implementation.

## Current implementation

| Topic | Guide |
|:--|:--|
| Serialized bytecode, stream parsing, and target conversion | [Bytecode](bytecode.md) |
| Dependency loading, initialization order, and symbol index | [Units](units.md) |
| Instruction words and operand coordinates | [Opword representation](opword.md) |
| Handler decoding, dispatch, calls, and supported operations | [Cold threaded interpreter](threaded-code.md) |
| Tagged values, stack frames, GC, and C runtime boundary | [Runtime](runtime.md) |
| Std functions, global variable, and native signatures | [Std runtime interface](std.md) |
| Differential runner, import fixtures, and result interpretation | [Testing](testing.md) |
| Build, sanitizers, formatting, and lint entry points | [Tooling](tooling.md) |

## Memory and future designs

| Topic | Guide | Status |
|:--|:--|:--|
| Packed layouts, alignment, storage, and object lifetime | [Packed memory](packed-memory.md) | General constraints |
| Native bridges and runtime symbol resolution | [Runtime bridges](runtime-bridges.md) | Explicit wrappers and generic bridge applied; symbol resolution deferred |
| In-place warming, unit context, and per-opcode layouts | [Warm interpreter](warm-interpreter.md) | Layout proposals in review |
| Linking decoded modules into a self-contained program | [Linking](linking.md) | Future design |
| Pointer enumeration and address rebasing | [Relocation](relocation.md) | Future mechanism |
| Specification and implementation gaps | [Deferred defects](deferred-defects.md) | Postponed |

The reference VM's [format specification](../../../virtual_machine/SPEC.md)
defines serialized bytecode. Its C implementation is a useful comparison, but
does not establish that either VM validates every malformed input correctly.
