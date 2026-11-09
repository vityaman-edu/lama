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

exec ctest \
  --test-dir "$vm_dir/build/$mode" \
  --output-on-failure \
  --no-tests=error \
  --label-regex '^unit$'
