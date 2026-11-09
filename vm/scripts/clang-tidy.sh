#!/usr/bin/env bash

set -euo pipefail

vm_dir=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)
mode=${1:-release}

while IFS= read -r -d '' file; do
  clang-tidy -p "$vm_dir/build/$mode" "$file"
done < <(find "$vm_dir/app" "$vm_dir/lib" "$vm_dir/test" \
  -type f -name '*.cpp' -print0)
