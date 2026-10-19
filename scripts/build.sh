#!/usr/bin/env bash
# Build Gladys (Release) and install it with its Qt/ICU runtime into dist/.
# Usage: scripts/build.sh [--ma-debug]
# Requires QT_ROOT_DIR (Qt install dir, used by CMake and *deployqt).
set -euo pipefail

cd "$(dirname "$0")/.."

# Clean dist/ but keep the downloaded llama.cpp and sherpaonnx folders.
[ -d dist ] && find dist -mindepth 1 -maxdepth 1 ! -name llama.cpp ! -name sherpaonnx -exec rm -rf {} +

MA_DEBUG=OFF
for arg in "$@"; do
  case "$arg" in
    --ma-debug) MA_DEBUG=ON ;;
    *) echo "Unknown option: $arg" >&2; exit 1 ;;
  esac
done

cmake --preset release -DGLADYS_MA_DEBUG="$MA_DEBUG"
cmake --build --preset release
cmake --install build/ --prefix "$PWD/dist"

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
      "$QT_ROOT_DIR/bin/linuxdeploy-x86_64.AppImage" --appdir dist --executable dist/gladys --plugin qt
    # Flatten usr/ back to dist/ root: the binary locates models next to itself
    mkdir -p dist/lib
    mv dist/usr/lib/* dist/lib/
    mv dist/usr/plugins dist/plugins
    mv dist/usr/qml dist/qml
    rm -rf dist/usr dist/AppRun
    printf '[Paths]\nPlugins = plugins\nQml2Imports = qml\n' > dist/qt.conf
    ;;
  Darwin) "$QT_ROOT_DIR/bin/macdeployqt" dist/gladys.app -qmldir=src/qml ;;
  *)      "$QT_ROOT_DIR/bin/windeployqt" --release --qmldir src/qml dist/gladys.exe ;;
esac
