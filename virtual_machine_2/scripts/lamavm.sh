#!/usr/bin/env bash
set -euo pipefail

vm_dir=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)
repo_dir=$(cd -- "$vm_dir/.." && pwd)

cd -- "$repo_dir"

if [[ $(uname -s) == Darwin ]]; then
  unset DEVELOPER_DIR SDKROOT
  export PATH="/usr/bin:$PATH"
  MACOS_SDK=$(/usr/bin/xcrun --sdk macosx --show-sdk-path)
  export MACOS_SDK SDKROOT="$MACOS_SDK"
fi

dune build virtual_machine/lama.exe
cat >"$vm_dir/lamavm" <<'SH'
#!/usr/bin/env bash
set -euo pipefail
repo_dir=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)
exec "$repo_dir/_build/default/virtual_machine/lama.exe" \
  -I "$repo_dir/_build/default/stdlib/x64" "$@"
SH
chmod +x "$vm_dir/lamavm"
