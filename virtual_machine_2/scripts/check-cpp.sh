#!/usr/bin/env bash
set -euo pipefail

vm_dir=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)
"$vm_dir/scripts/lamavm2.sh"

while IFS= read -r -d '' file; do
  clang-format --dry-run --Werror "$file"
done < <(find "$vm_dir/source" -type f \( -name '*.c' -o -name '*.cpp' -o -name '*.h' -o -name '*.hpp' \) -print0)

find "$vm_dir/source" -type f \( -name '*.c' -o -name '*.cpp' \) -print0 |
  xargs -0 -n 1 -P 8 bash -c '
    printf "[clang-tidy] %s\n" "$2"
    exec "$1/scripts/clang-tidy.sh" -p "$1/build/rel" "$2"
  ' _ "$vm_dir"
