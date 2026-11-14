# Readable compiler disassembly

Applied scope: format compiler disassembly in test failure reports.

When `--disassemble` is enabled and a test fails, print the compiler listing
under `lamac disassembly: |2`. Decode UTF-8 with replacement for invalid bytes
and indent each listing line by two spaces relative to the key. Preserve the
existing reference VM disassembly report.

Validate with the sanitized trace build and `import/Sum4*` filter, two runs,
and `--disassemble`. Run the Python style checks.
