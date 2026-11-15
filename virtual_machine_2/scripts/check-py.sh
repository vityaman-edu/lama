#!/usr/bin/env bash
set -euo pipefail

vm_dir=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)
cd -- "$vm_dir"
ruff check --no-cache test source/interpreter/std/std.py
ruff format --no-cache --check test source/interpreter/std/std.py
