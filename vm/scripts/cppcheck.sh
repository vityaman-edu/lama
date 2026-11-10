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

build_dir="$vm_dir/build/$mode"

# Normalize an analysis-only copy; Cppcheck's native importer ignores -isystem paths.
sed 's/-isystem /-I /g' "$build_dir/compile_commands.json" \
  > "$build_dir/cppcheck_compile_commands.json"

exec cppcheck \
  --enable=all \
  --check-level=exhaustive \
  --error-exitcode=1 \
  --std=c++23 \
  --language=c++ \
  --library=googletest \
  --library="$vm_dir/scripts/cppcheck/test-frameworks.cfg" \
  --suppress=missingIncludeSystem \
  --project="$build_dir/cppcheck_compile_commands.json" \
  --file-filter="$vm_dir/app/**" \
  --file-filter="$vm_dir/lib/**" \
  --file-filter="$vm_dir/test/**"
