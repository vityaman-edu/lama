#!/usr/bin/env bash

set -euo pipefail

vm_dir=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)
mode=${1:-release}

if [ "$#" -gt 1 ]; then
  printf 'Usage: %s [release|asan|ubsan|msan|fuzz]\n' "$0" >&2
  exit 2
fi

case "$mode" in
  release|asan|ubsan|msan|fuzz) ;;
  *) printf 'Usage: %s [release|asan|ubsan|msan|fuzz]\n' "$0" >&2; exit 2 ;;
esac

"$vm_dir/scripts/clang-format.sh" --dry-run --Werror
"$vm_dir/scripts/clang-tidy.sh" "$mode"
"$vm_dir/scripts/cppcheck.sh" "$mode"

cd "$vm_dir/.."
dune build @@regression/runtest

python3 "$vm_dir/test/golden-legacy/main.py" \
  --lamavm "$vm_dir/build/$mode/lamavm"

"$vm_dir/scripts/googletest.sh" "$mode"
"$vm_dir/scripts/fuzztest.sh" "$mode"
