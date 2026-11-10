#!/usr/bin/env bash

set -euo pipefail

repo_dir=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/../.." && pwd)

compiler=("$repo_dir/_build/default/src/Driver.exe" \
  -runtime "$repo_dir/runtime" \
  -I "$repo_dir/_build/default/stdlib/x64")
better=false

for arg in "$@"; do
  if [[ "$arg" == --better ]]; then
    better=true
  fi
done

if ! "$better"; then
  exec "${compiler[@]}" "$@"
fi

args=(-b -ds)
path_argument=false

for arg in "$@"; do
  if "$path_argument" || [[ "$arg" != -* ]]; then
    if [[ "$arg" != /* ]]; then
      arg="$PWD/$arg"
    fi

    path_argument=false
  else
    case "$arg" in
      --better|-b|-ds) continue ;;
      -I|-runtime|-o) path_argument=true ;;
    esac
  fi

  args+=("$arg")
done

tmpdir=$(mktemp -d "${TMPDIR:-/tmp}/lamac.XXXXXX")
trap 'rm -rf -- "$tmpdir"' EXIT

(
  cd -- "$tmpdir"
  "${compiler[@]}" "${args[@]}" >&2
)

cat -- "$tmpdir"/*.bc
cat -- "$tmpdir"/*.sm >&2
