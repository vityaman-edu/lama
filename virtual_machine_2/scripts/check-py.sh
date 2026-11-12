#!/usr/bin/env bash
set -euo pipefail

vm_dir=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)
cd -- "$vm_dir"
ruff check --no-cache test
ruff format --no-cache --check test
