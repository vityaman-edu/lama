#!/usr/bin/env bash

set -euo pipefail

vm_dir=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd -P)

cd "$vm_dir"

ruff check \
  --no-cache \
  --config "$vm_dir/ruff.toml" \
  "$vm_dir/scripts" \
  "$vm_dir/test"

exec ruff format \
  --check \
  --no-cache \
  --config "$vm_dir/ruff.toml" \
  "$vm_dir/scripts" \
  "$vm_dir/test"
