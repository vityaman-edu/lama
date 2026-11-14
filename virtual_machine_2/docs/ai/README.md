# VM design notes

Notes from the discussion on 2026-11-14. These record requirements, findings,
proposals, and open questions. They do not apply the proposed designs or replace
the existing implementation specifications.

| Topic | Note | Status |
|:--|:--|:--|
| Writable byte storage | [Buffer](buffer.md) | Design postponed |
| Packed layouts, alignment, and object lifetime | [Packed memory](packed-memory.md) | Explanation and compiler checks |
| Memory interpretation in `unit::parse` | [Unit memory review](unit-memory-review.md) | Findings, reproductions, and fix status |
| Aligned instruction words and dispatch | [Opword representation](opword.md) | Proposed execution representation |
| Independent decoding and dependency closure | [Linking](linking.md) | Proposed construction pipeline |
| Pointer rebasing and region lookup | [Relocation](relocation.md) | Proposed mechanism and constraints |

The earlier [decoder specification](decode.md) and
[execution integration specification](execution-decode.md) describe the existing
packed-instruction implementation. The new execution and linking proposals have
not superseded them.

The reference VM's [format specification](../../../virtual_machine/SPEC.md)
defines the serialized bytecode. Its C implementation is a reference to inspect,
not a guarantee that every validation check is correct.
