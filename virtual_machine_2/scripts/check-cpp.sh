#!/usr/bin/env bash
set -euo pipefail

vm_dir=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)
exit_status=0
"$vm_dir/scripts/lamavm2.sh" || exit_status=1

while IFS= read -r -d '' file; do
  clang-format --dry-run --Werror "$file" || exit_status=1
done < <(find "$vm_dir/source" -type f \( -name '*.c' -o -name '*.cpp' -o -name '*.h' -o -name '*.hpp' \) -print0)

find "$vm_dir/source" -type f \( -name '*.c' -o -name '*.cpp' \) -print0 |
  xargs -0 -n 1 -P 8 bash -c '
    printf "[clang-tidy] %s\n" "$2"
    "$1/scripts/clang-tidy.sh" -p "$1/build/rel" "$2" || exit 1
  ' _ "$vm_dir" || exit_status=1

exit "$exit_status"
