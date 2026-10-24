#!/usr/bin/env bash
# Install Qt 6.8.3 desktop into $GLADYS_SPACE/qt-6.8.3/ and print the QT_ROOT_DIR export.
# Usage: scripts/qt-install.sh <linux|macos|windows>
set -euo pipefail

OS=${1:?usage: qt-install.sh <linux|macos|windows>}
QT_VER=6.8.3
SPACE=${GLADYS_SPACE:-$HOME/gladys}
QT_DIR="$SPACE/qt-$QT_VER"

echo "GLADYS_SPACE selected: $SPACE"

case "$OS" in
  linux)   HOST=linux;   ARCH=linux_gcc_64;      DIR=gcc_64 ;;
  macos)   HOST=mac;     ARCH=clang_64;          DIR=macos ;;
  windows) HOST=windows; ARCH=win64_msvc2022_64; DIR=msvc2022_64 ;;
  *) echo "Unknown OS: $OS" >&2; exit 1 ;;
esac

case "$OS:$(uname -s)" in
  linux:Linux|macos:Darwin|windows:MINGW*|windows:MSYS*|windows:CYGWIN*) ;;
  *) echo "Host is $(uname -s), not $OS" >&2; exit 1 ;;
esac

command -v aqt >/dev/null || { echo "aqtinstall not found: pip install aqtinstall" >&2; exit 1; }

mkdir -p "$SPACE" && [ -w "$SPACE" ] || { echo "Build space not writable: $SPACE" >&2; exit 1; }
[ ! -e "$QT_DIR/$DIR" ] || { echo "Already installed: $QT_DIR/$DIR" >&2; exit 1; }

aqt install-qt -O "$QT_DIR" "$HOST" desktop "$QT_VER" "$ARCH"

# aqt nests the install as <outputdir>/<version>/<arch>; flatten it to <outputdir>/<arch>.
mv "$QT_DIR/$QT_VER/$DIR" "$QT_DIR/$DIR"
rmdir "$QT_DIR/$QT_VER"

echo "export QT_ROOT_DIR=$QT_DIR/$DIR"
