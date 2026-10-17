#!/usr/bin/env bash
# Optional unprivileged Linux x64 installation, also used in the cloud environment.
set -euo pipefail
if [[ "$(uname -s)" != Linux || "$(uname -m)" != x86_64 ]]; then
  echo 'This helper requires Linux x86_64. Use the native installer on Windows.' >&2
  exit 1
fi
command -v curl >/dev/null
command -v zstd >/dev/null
command -v sha256sum >/dev/null
install_dir="${1:-$PWD/.cache/ollama}"
if [[ -x "$install_dir/bin/ollama" ]]; then
  echo "Ollama already installed at $install_dir; leaving it unchanged."
  exit 0
fi
mkdir -p "$install_dir"
task_tmp="$(mktemp -d)"
trap 'rm -rf "$task_tmp"' EXIT
archive=ollama-linux-amd64.tar.zst
curl --fail --location --retry 2 "https://github.com/ollama/ollama/releases/download/v0.40.5/$archive" -o "$task_tmp/$archive"
# Official release sha256sum.txt, verified during initial setup.
expected=dc5eabc292f9949546c90a97c820f924b035e05243b0de13c7f366182562169c
(cd "$task_tmp" && echo "$expected  $archive" | sha256sum --check --strict)
tar --zstd -xf "$task_tmp/$archive" -C "$install_dir"
echo "Installed Ollama v0.40.5 in $install_dir"
