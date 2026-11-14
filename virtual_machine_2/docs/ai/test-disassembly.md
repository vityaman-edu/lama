# Test disassembly reporting

Applied scope: omit compiler disassembly from test reports.

Do not print compiler disassembly, including when `--disassemble` is supplied.
Preserve the existing reference VM disassembly report on failures.

Validate with the sanitized trace build and `import/Sum4*` filter, two runs,
and `--disassemble`. Run the Python style checks.
