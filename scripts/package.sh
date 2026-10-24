#!/usr/bin/env bash
# Package $GLADYS_SPACE/dist (built by scripts/build.sh) with models into $GLADYS_SPACE/release/.
# Usage: scripts/package.sh <linux|macos|windows> <arch> <version>
set -euo pipefail

cd "$(dirname "$0")/.."

SPACE=${GLADYS_SPACE:-$HOME/gladys}
DIST="$SPACE/dist"
RELEASE="$SPACE/release"

OS=${1:?usage: package.sh <linux|macos|windows> <arch> <version>}
ARCH=${2:?missing arch}
VERSION=${3:?missing version}

case "$OS" in
  linux|macos|windows) ;;
  *) echo "Unknown OS: $OS" >&2; exit 1 ;;
esac

scripts/fetch-deps.sh

mkdir -p "$RELEASE"
EXCLUDE=(-x '*.h' '*.hpp' '*.sh' '*miniaudio*')

# ---- Archive dist/ ----
case "$OS" in
  linux)   tar -C "$DIST" --exclude='*.h' --exclude='*.hpp' --exclude='*.sh' --exclude='*miniaudio*' -czf "$RELEASE/gladys-linux-$ARCH-$VERSION.tar.gz" . ;;
  macos)   (cd "$DIST" && zip -qry "$RELEASE/gladys-macos-$ARCH-$VERSION.zip" . "${EXCLUDE[@]}") ;;
  windows) (cd "$DIST" && zip -qr "$RELEASE/gladys-win-$ARCH-$VERSION.zip" . "${EXCLUDE[@]}") ;;
esac
