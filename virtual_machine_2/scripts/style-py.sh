#!/usr/bin/env bash
set -euo pipefail

vm_dir=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)
cd -- "$vm_dir"
ruff format --no-cache test
ruff check --no-cache --fix test
