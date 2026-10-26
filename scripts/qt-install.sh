#!/usr/bin/env bash
# Install Qt 6.8.3 desktop into $GLADYS_SPACE/qt-6.8.3/ and print the QT_ROOT_DIR export.
# Usage: scripts/qt-install.sh <linux|macos|windows> [x64|arm64]
set -euo pipefail

usage() {
  echo "Usage: scripts/qt-install.sh <linux|macos|windows> [x64|arm64]" >&2;
  exit 1;
}

[ $# -ge 1 ] && [ $# -le 2 ] || usage

OS=$1
case "$(uname -s)" in
  Darwin) DEFAULT_ARCH=arm64 ;;
  *) DEFAULT_ARCH=x64 ;;
esac

ARCH=${2:-$DEFAULT_ARCH}
case "$ARCH" in
  x64|arm64) ;;
  *) usage ;;
esac

QT_VER=6.8.3
SPACE=${GLADYS_SPACE:-$HOME/gladys}
QT_DIR="$SPACE/qt-$QT_VER"

echo "GLADYS_SPACE selected: $SPACE"

case "$OS-$ARCH" in
  linux-x64)     HOST=linux;         AQT_ARCH=linux_gcc_64;         DIR=gcc_64 ;;
  linux-arm64)   HOST=linux_arm64;   AQT_ARCH=linux_gcc_arm64;      DIR=gcc_arm64 ;;
  macos-*)       HOST=mac;           AQT_ARCH=clang_64;             DIR=macos ;;
  windows-x64)   HOST=windows;       AQT_ARCH=win64_msvc2022_64;    DIR=msvc2022_64 ;;
  windows-arm64) HOST=windows_arm64; AQT_ARCH=win64_msvc2022_arm64; DIR=msvc2022_arm64 ;;
  *) echo "Unsupported target: $OS $ARCH" >&2; exit 1 ;;
esac

case "$OS:$(uname -s)" in
  linux:Linux|macos:Darwin|windows:MINGW*|windows:MSYS*|windows:CYGWIN*) ;;
  *) echo "Host $(uname -s) cannot install Qt for $OS" >&2; exit 1 ;;
esac

case "$ARCH:$(uname -m)" in
  x64:x86_64|arm64:aarch64|arm64:arm64) ;;
  *) echo "Host $(uname -m) cannot install Qt for $ARCH" >&2; exit 1 ;;
esac

command -v aqt >/dev/null || { echo "aqtinstall not found: pip install aqtinstall" >&2; exit 1; }

mkdir -p "$SPACE" && [ -w "$SPACE" ] || { echo "Build space not writable: $SPACE" >&2; exit 1; }
[ ! -e "$QT_DIR/$DIR" ] || { echo "Already installed: $QT_DIR/$DIR" >&2; exit 1; }

aqt install-qt -O "$QT_DIR" "$HOST" desktop "$QT_VER" "$AQT_ARCH"

# aqt nests the install as <outputdir>/<version>/<arch>; flatten it to <outputdir>/<arch>.
mv "$QT_DIR/$QT_VER/$DIR" "$QT_DIR/$DIR"
rmdir "$QT_DIR/$QT_VER"

echo "export QT_ROOT_DIR=$QT_DIR/$DIR"
