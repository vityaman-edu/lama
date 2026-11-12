#!/usr/bin/env bash
set -euo pipefail

mode=rel

case ${1:-} in
  rel|san) mode=$1; shift ;;
esac

vm_dir=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)

if [[ $mode == san ]]; then
  export UBSAN_OPTIONS="${UBSAN_OPTIONS:-halt_on_error=1:print_stacktrace=1}"

  if [[ $(uname -s) == Darwin ]]; then
    export MallocNanoZone=0
  fi
fi

exec python3 -B "$vm_dir/test/main.py" --lamavm "$vm_dir/build/$mode/lamavm2" "$@"
