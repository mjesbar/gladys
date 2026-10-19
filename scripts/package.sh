#!/usr/bin/env bash
# Package dist/ (built by scripts/build.sh) with models into release/.
# Usage: scripts/package.sh <linux|macos|windows> <arch> <version>
set -euo pipefail

cd "$(dirname "$0")/.."

OS=${1:?usage: package.sh <linux|macos|windows> <arch> <version>}
ARCH=${2:?missing arch}
VERSION=${3:?missing version}

case "$OS" in
  linux|macos|windows) ;;
  *) echo "Unknown OS: $OS" >&2; exit 1 ;;
esac

scripts/fetch-deps.sh

mkdir -p release
EXCLUDE=(-x '*.h' '*.hpp' '*.sh' '*miniaudio*')

# ---- Archive dist/ ----
case "$OS" in
  linux)   tar -C dist --exclude='*.h' --exclude='*.hpp' --exclude='*.sh' --exclude='*miniaudio*' -czf "release/gladys-linux-$ARCH-$VERSION.tar.gz" . ;;
  macos)   (cd dist && zip -qry "../release/gladys-macos-$ARCH-$VERSION.zip" . "${EXCLUDE[@]}") ;;
  windows) (cd dist && zip -qr "../release/gladys-win-$ARCH-$VERSION.zip" . "${EXCLUDE[@]}") ;;
esac
