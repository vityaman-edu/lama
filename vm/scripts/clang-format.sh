#!/usr/bin/env bash

set -euo pipefail

vm_dir=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)
files=()

while IFS= read -r -d '' file; do
  files+=("$file")
done < <(find "$vm_dir/app" "$vm_dir/lib" "$vm_dir/test" \
  -type f \( -name '*.cpp' -o -name '*.hpp' \) -print0)

if [ "$#" -eq 0 ]; then
  set -- -i
fi

clang-format "$@" "${files[@]}"
