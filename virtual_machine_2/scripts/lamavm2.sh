#!/usr/bin/env bash
set -euo pipefail

mode=${1:-rel}
target=lamavm2

if [[ $mode == cold ]]; then
  mode=rel
  target=lamavm2-cold
  shift
  set -- "$mode" "$@"
fi

if [[ $mode == san && -n ${NIX_HARDENING_ENABLE:-} ]]; then
  hardening_flags=" $NIX_HARDENING_ENABLE "
  hardening_flags=${hardening_flags// fortify / }
  export NIX_HARDENING_ENABLE=${hardening_flags// fortify3 / }
fi

if [[ $(uname -s) == Darwin ]]; then
  unset DEVELOPER_DIR SDKROOT
fi

vm_dir=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)
"$vm_dir/scripts/configure.sh" "$@"
cmake --build "$vm_dir/build/$mode" --target "$target" --parallel "${LAMAVM_BUILD_JOBS:-2}"
ln -sfn "build/$mode/$target" "$vm_dir/$target"
