#!/usr/bin/env bash
set -euo pipefail

vm_dir=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)
"$vm_dir/scripts/lamavm2.sh"

while IFS= read -r -d '' file; do
  clang-format --dry-run --Werror "$file"
done < <(find "$vm_dir/source" -type f \( -name '*.c' -o -name '*.cpp' -o -name '*.h' -o -name '*.hpp' \) -print0)

while IFS= read -r -d '' file; do
  "$vm_dir/scripts/clang-tidy.sh" -p "$vm_dir/build/rel" "$file"
done < <(find "$vm_dir/source" -type f \( -name '*.c' -o -name '*.cpp' \) -print0)
