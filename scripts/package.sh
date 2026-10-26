#!/usr/bin/env bash
# Package $GLADYS_SPACE/dist (built by scripts/build.sh) with models into $GLADYS_SPACE/release/.
# Usage: scripts/package.sh <linux|macos|windows> <x64|arm64> <version>
set -euo pipefail

cd "$(dirname "$0")/.."

SPACE=${GLADYS_SPACE:-$HOME/gladys}
DIST="$SPACE/dist"
RELEASE="$SPACE/release"

usage() {
  echo "Usage: scripts/package.sh <linux|macos|windows> <x64|arm64> <version>" >&2;
  exit 1;
}

[ $# -eq 3 ] || usage

OS=$1
ARCH=$2
VERSION=$3

case "$OS:$ARCH" in
  linux:x64|linux:arm64|windows:x64|windows:arm64|macos:x64|macos:arm64) ;;
  *) usage ;;
esac

scripts/fetch-deps.sh "$ARCH"

mkdir -p "$RELEASE"
EXCLUDE=(-x '*.h' '*.hpp' '*.sh' '*miniaudio*')

# ---- Archive dist/ ----
case "$OS" in
  linux)   tar -C "$DIST" --exclude='*.h' --exclude='*.hpp' --exclude='*.sh' --exclude='*miniaudio*' -czf "$RELEASE/gladys-linux-$ARCH-$VERSION.tar.gz" . ;;
  macos)   (cd "$DIST" && zip -qry "$RELEASE/gladys-macos-$ARCH-$VERSION.zip" . "${EXCLUDE[@]}") ;;
  windows) (cd "$DIST" && zip -qr "$RELEASE/gladys-win-$ARCH-$VERSION.zip" . "${EXCLUDE[@]}") ;;
esac
