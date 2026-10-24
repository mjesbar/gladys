#!/usr/bin/env bash
# Build Gladys (Release) in $GLADYS_SPACE and install it with its Qt/ICU runtime into $GLADYS_SPACE/dist/.
# Usage: scripts/build.sh [x64|arm64] [on|off]   (2nd arg: miniaudio debug output)
# Requires QT_ROOT_DIR (Qt install dir, used by CMake and *deployqt).
set -euo pipefail

usage() { echo "Usage: scripts/build.sh [x64|arm64] [on|off]" >&2; exit 1; }
[ $# -le 2 ] || usage

case "$(uname -s)" in Darwin) DEFAULT_ARCH=arm64 ;; *) DEFAULT_ARCH=x64 ;; esac
ARCH=${1:-$DEFAULT_ARCH}
MA_DEBUG=${2:-off}
case "$ARCH" in x64|arm64) ;; *) usage ;; esac
case "$MA_DEBUG" in on) MA_DEBUG=ON ;; off) MA_DEBUG=OFF ;; *) usage ;; esac

cd "$(dirname "$0")/.."

: "${QT_ROOT_DIR:?set QT_ROOT_DIR to the Qt arch folder first}"
SPACE=${GLADYS_SPACE:-$HOME/gladys}
DIST="$SPACE/dist"
BUILD="$SPACE/build"

# Clean dist/ but keep the downloaded llama.cpp and sherpaonnx folders.
[ -d "$DIST" ] && find "$DIST" -mindepth 1 -maxdepth 1 ! -name llama.cpp ! -name sherpaonnx -exec rm -rf {} +

cmake --preset release -B "$BUILD" -DGLADYS_ARCH="$ARCH" -DGLADYS_MA_DEBUG="$MA_DEBUG"
cmake --build "$BUILD" --config Release
cmake --install "$BUILD" --prefix "$DIST"

# Bundle the Qt runtime into dist/
case "$(uname -s)" in
  Linux)
    for tool in linuxdeploy linuxdeploy-plugin-qt; do
      BIN="$QT_ROOT_DIR/bin/$tool-x86_64.AppImage"
      [ -x "$BIN" ] || {
        curl -fL -o "$BIN" "https://github.com/linuxdeploy/$tool/releases/download/continuous/$tool-x86_64.AppImage"
        chmod +x "$BIN"
      }
    done
    LD_LIBRARY_PATH="$QT_ROOT_DIR/lib" QMAKE="$QT_ROOT_DIR/bin/qmake" QML_SOURCES_PATHS=src/qml \
      "$QT_ROOT_DIR/bin/linuxdeploy-x86_64.AppImage" --appdir "$DIST" --executable "$DIST/gladys" --plugin qt
    # Flatten usr/ back to dist/ root: the binary locates models next to itself
    mkdir -p "$DIST/lib"
    mv "$DIST"/usr/lib/* "$DIST/lib/"
    mv "$DIST/usr/plugins" "$DIST/plugins"
    mv "$DIST/usr/qml" "$DIST/qml"
    rm -rf "$DIST/usr" "$DIST/AppRun"
    printf '[Paths]\nPlugins = plugins\nQml2Imports = qml\n' > "$DIST/qt.conf"
    ;;
  Darwin) "$QT_ROOT_DIR/bin/macdeployqt" "$DIST/gladys.app" -qmldir=src/qml ;;
  *)      "$QT_ROOT_DIR/bin/windeployqt" --release --qmldir src/qml "$DIST/gladys.exe" ;;
esac
