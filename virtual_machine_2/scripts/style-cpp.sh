#!/usr/bin/env bash
set -euo pipefail

vm_dir=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)
"$vm_dir/scripts/lamavm2.sh"

while IFS= read -r -d '' file; do
  clang-tidy -p "$vm_dir/build/rel" -fix "$file"
done < <(find "$vm_dir/source" -type f \( -name '*.c' -o -name '*.cpp' \) -print0)

while IFS= read -r -d '' file; do
  clang-format -i "$file"
done < <(find "$vm_dir/source" -type f \( -name '*.c' -o -name '*.cpp' -o -name '*.h' -o -name '*.hpp' \) -print0)
