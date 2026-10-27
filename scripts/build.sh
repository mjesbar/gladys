#!/usr/bin/env bash
# Build Gladys (Release) in $GLADYS_SPACE and install it with its Qt/ICU runtime into $GLADYS_SPACE/dist/.
# Usage: scripts/build.sh [--os linux|macos|windows] [--arch x64|arm64] [--ma-debug on|off]
# Requires QT_ROOT_DIR (Qt install dir).
set -euo pipefail

usage() {
  echo "Usage: scripts/build.sh [--os linux|macos|windows] [--arch x64|arm64] [--ma-debug on|off]" >&2;
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
MA_DEBUG=off
while [ $# -gt 0 ]; do
  [ $# -ge 2 ] || usage
  case "$1" in
    --os) OS=$2 ;;
    --arch) ARCH=$2 ;;
    --ma-debug) MA_DEBUG=$2 ;;
    *) usage ;;
  esac
  shift 2
done

case "$OS" in
  linux|macos|windows) ;;
  *) usage ;;
esac
case "$ARCH" in
  x64|arm64) ;;
  *) usage ;;
esac
case "$MA_DEBUG" in
  on) MA_DEBUG=ON ;;
  off) MA_DEBUG=OFF ;;
  *) usage ;;
esac
[ "$OS" = "$HOST_OS" ] || { echo "Target $OS cannot be built on host $HOST_OS" >&2; exit 1; }

cd "$(dirname "$0")/.."

: "${QT_ROOT_DIR:?set QT_ROOT_DIR to the Qt arch folder first}"
SPACE=${GLADYS_SPACE:-$HOME/gladys}
DIST="$SPACE/dist"
BUILD="$SPACE/build"

# Bring deps back from the bundle (macOS, see below) so the clean step keeps them.
for dep in sherpaonnx llama.cpp; do
  for loc in MacOS Resources; do
    [ -d "$DIST/gladys.app/Contents/$loc/$dep" ] && mv "$DIST/gladys.app/Contents/$loc/$dep" "$DIST/$dep"
  done
done

# Clean dist/ but keep the downloaded llama.cpp and sherpaonnx folders.
echo "[build] clean dist ..."
[ -d "$DIST" ] && find "$DIST" -mindepth 1 -maxdepth 1 ! -name llama.cpp ! -name sherpaonnx -exec rm -rf {} +
echo "[build] clean dist OK"

echo "[build] cmake configure ($OS $ARCH) ..."
cmake --preset release -B "$BUILD" -DGLADYS_ARCH="$ARCH" -DGLADYS_MA_DEBUG="$MA_DEBUG"
echo "[build] cmake configure OK"

echo "[build] cmake build ..."
cmake --build "$BUILD" --config Release
echo "[build] cmake build OK"

echo "[build] cmake install ..."
cmake --install "$BUILD" --prefix "$DIST"
echo "[build] cmake install OK"

# Bundle the Qt runtime into dist/
case "$OS" in
  linux)
    for tool in linuxdeploy linuxdeploy-plugin-qt; do
      BIN="$QT_ROOT_DIR/bin/$tool-x86_64.AppImage"
      [ -x "$BIN" ] || {
        echo "[build] download $tool ..."
        curl -fL -o "$BIN" "https://github.com/linuxdeploy/$tool/releases/download/continuous/$tool-x86_64.AppImage"
        chmod +x "$BIN"
        echo "[build] download $tool OK"
      }
    done
    echo "[build] linuxdeploy bundle Qt ..."
    LD_LIBRARY_PATH="$QT_ROOT_DIR/lib" QMAKE="$QT_ROOT_DIR/bin/qmake" QML_SOURCES_PATHS=src/qml \
      "$QT_ROOT_DIR/bin/linuxdeploy-x86_64.AppImage" --appdir "$DIST" --executable "$DIST/gladys" --plugin qt
    # Flatten usr/ back to dist/ root: the binary locates models next to itself
    mkdir -p "$DIST/lib"
    mv "$DIST"/usr/lib/* "$DIST/lib/"
    mv "$DIST/usr/plugins" "$DIST/plugins"
    mv "$DIST/usr/qml" "$DIST/qml"
    rm -rf "$DIST/usr" "$DIST/AppRun"
    printf '[Paths]\nPlugins = plugins\nQml2Imports = qml\n' > "$DIST/qt.conf"
    echo "[build] linuxdeploy bundle Qt OK"
    ;;
  macos)
    echo "[build] macdeployqt bundle Qt ..."
    "$QT_ROOT_DIR/bin/macdeployqt" "$DIST/gladys.app" -qmldir=src/qml
    echo "[build] macdeployqt bundle Qt OK"
    # Without this key macOS never lists the app in Privacy > Microphone and capture gets silence.
    /usr/libexec/PlistBuddy -c "Add :NSMicrophoneUsageDescription string 'Gladys records your voice for speech-to-text.'" "$DIST/gladys.app/Contents/Info.plist"
    for dep in sherpaonnx llama.cpp; do
      [ -d "$DIST/$dep" ] && mv "$DIST/$dep" "$DIST/gladys.app/Contents/MacOS/"
    done
    ;;
  windows)
    echo "[build] windeployqt bundle Qt ..."
    "$QT_ROOT_DIR/bin/windeployqt" --release --qmldir src/qml "$DIST/gladys.exe"
    echo "[build] windeployqt bundle Qt OK"
    ;;
esac
