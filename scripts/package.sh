#!/usr/bin/env bash
# Package $GLADYS_SPACE/dist (built by scripts/build.sh) with models into $GLADYS_SPACE/release/.
# Usage: scripts/package.sh [--os linux|macos|windows] [--arch x64|arm64] --version <v>
set -euo pipefail

cd "$(dirname "$0")/.."

SPACE=${GLADYS_SPACE:-$HOME/gladys}
DIST="$SPACE/dist"
RELEASE="$SPACE/release"

usage() {
  echo "Usage: scripts/package.sh [--os linux|macos|windows] [--arch x64|arm64] --version <v>" >&2;
  exit 1;
}

case "$(uname -s)" in
  Linux) HOST_OS=linux; DEFAULT_ARCH=x64 ;;
  Darwin) HOST_OS=macos; DEFAULT_ARCH=arm64 ;;
  MINGW*|MSYS*|CYGWIN*) HOST_OS=windows; DEFAULT_ARCH=x64 ;;
  *) echo "Unsupported host: $(uname -s)" >&2; exit 1 ;;
esac

OS=$HOST_OS
ARCH=$DEFAULT_ARCH
VERSION=
while [ $# -gt 0 ]; do
  [ $# -ge 2 ] || usage
  case "$1" in
    --os) OS=$2 ;;
    --arch) ARCH=$2 ;;
    --version) VERSION=$2 ;;
    *) usage ;;
  esac
  shift 2
done

[ -n "$VERSION" ] || usage
case "$OS:$ARCH" in
  linux:x64|windows:x64|macos:arm64) ;;
  *) usage ;;
esac
[ "$OS" = "$HOST_OS" ] || { echo "Target $OS cannot be packaged on host $HOST_OS" >&2; exit 1; }

scripts/fetch-deps.sh --os "$OS" --arch "$ARCH"

mkdir -p "$RELEASE"
EXCLUDE=(-x '*.h' '*.hpp' '*.sh' '*miniaudio*')

# ---- Archive dist/ ----
echo "[package] archive $OS $ARCH $VERSION ..."
case "$OS" in
  linux)   tar -C "$DIST" --exclude='*.h' --exclude='*.hpp' --exclude='*.sh' --exclude='*miniaudio*' -czf "$RELEASE/gladys-linux-$ARCH-$VERSION.tar.gz" . ;;
  macos)   (cd "$DIST" && zip -qry "$RELEASE/gladys-macos-$ARCH-$VERSION.zip" . "${EXCLUDE[@]}") ;;
  windows) (cd "$DIST" && zip -qr "$RELEASE/gladys-win-$ARCH-$VERSION.zip" . "${EXCLUDE[@]}") ;;
esac
echo "[package] archive $OS $ARCH $VERSION OK"
