#!/usr/bin/env bash

set -euo pipefail

repo_dir=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/../.." && pwd)

exec "$repo_dir/_build/default/src/Driver.exe" \
  -runtime "$repo_dir/runtime" \
  -I "$repo_dir/_build/default/stdlib/x64" \
  "$@"
