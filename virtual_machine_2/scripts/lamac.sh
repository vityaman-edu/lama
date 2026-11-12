#!/usr/bin/env bash
set -euo pipefail

vm_dir=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)
repo_dir=$(cd -- "$vm_dir/.." && pwd)

cd -- "$repo_dir"
dune build src/Driver.exe
ln -sfn ../_build/default/src/Driver.exe "$vm_dir/lamac"
