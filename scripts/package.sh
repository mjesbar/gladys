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

# ---- Clean dist/ for release ----
find dist -type f \( -name '*.tar.gz' -o -name '*.tar.bz2' -o -name '*.tar.xz' -o -name '*.zip' \) -delete

# ---- Stage and archive ----
STAGE=release/gladys-v$VERSION
mkdir -p "$STAGE"
rsync -a \
  --exclude='*.h' \
  --exclude='*.hpp' \
  --exclude='*.sh' \
  --exclude=miniaudio \
  dist/ "$STAGE/"

case "$OS" in
  linux)   tar -C release -czf "release/gladys-linux-$ARCH-$VERSION.tar.gz" "gladys-v$VERSION" ;;
  macos)   (cd release && zip -r "gladys-macos-$ARCH-$VERSION.zip" "gladys-v$VERSION") ;;
  windows) (cd release && zip -r "gladys-win-$ARCH-$VERSION.zip" "gladys-v$VERSION") ;;
esac
