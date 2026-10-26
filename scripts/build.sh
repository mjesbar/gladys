#!/usr/bin/env bash
# Build Gladys (Release) in $GLADYS_SPACE and install it with its Qt/ICU runtime into $GLADYS_SPACE/dist/.
# Usage: scripts/build.sh [--os linux|macos|windows] [--arch x64|arm64] [--ma-debug on|off] [--identity <hash>]
# Requires QT_ROOT_DIR (Qt install dir). --identity is required with --os macos (codesign identity).
set -euo pipefail

usage() {
  echo "Usage: scripts/build.sh [--os linux|macos|windows] [--arch x64|arm64] [--ma-debug on|off] [--identity <hash>]" >&2;
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
IDENTITY=
while [ $# -gt 0 ]; do
  [ $# -ge 2 ] || usage
  case "$1" in
    --os) OS=$2 ;;
    --arch) ARCH=$2 ;;
    --ma-debug) MA_DEBUG=$2 ;;
    --identity) IDENTITY=$2 ;;
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
if [ "$OS" = macos ]; then
  [ -n "$IDENTITY" ] || usage
elif [ -n "$IDENTITY" ]; then
  usage
fi

cd "$(dirname "$0")/.."

: "${QT_ROOT_DIR:?set QT_ROOT_DIR to the Qt arch folder first}"
SPACE=${GLADYS_SPACE:-$HOME/gladys}
DIST="$SPACE/dist"
BUILD="$SPACE/build"

# Bring deps back from the bundle (macOS, see below) so the clean step keeps them.
for dep in sherpaonnx llama.cpp; do
  [ -d "$DIST/gladys.app/Contents/MacOS/$dep" ] && mv "$DIST/gladys.app/Contents/MacOS/$dep" "$DIST/$dep"
done

# Clean dist/ but keep the downloaded llama.cpp and sherpaonnx folders.
[ -d "$DIST" ] && find "$DIST" -mindepth 1 -maxdepth 1 ! -name llama.cpp ! -name sherpaonnx -exec rm -rf {} +

cmake --preset release -B "$BUILD" -DGLADYS_ARCH="$ARCH" -DGLADYS_MA_DEBUG="$MA_DEBUG"
cmake --build "$BUILD" --config Release
cmake --install "$BUILD" --prefix "$DIST"

# Bundle the Qt runtime into dist/
case "$OS" in
  linux)
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
  macos)
    "$QT_ROOT_DIR/bin/macdeployqt" "$DIST/gladys.app" -qmldir=src/qml
    # Without this key macOS never lists the app in Privacy > Microphone and capture gets silence.
    /usr/libexec/PlistBuddy -c "Add :NSMicrophoneUsageDescription string 'Gladys records your voice for speech-to-text.'" "$DIST/gladys.app/Contents/Info.plist"
    # The app looks for models next to its binary: move them into the bundle.
    for dep in sherpaonnx llama.cpp; do
      [ -d "$DIST/$dep" ] && mv "$DIST/$dep" "$DIST/gladys.app/Contents/MacOS/"
    done
    # Stable signature: TCC keeps Accessibility/Input Monitoring across rebuilds. Sign last.
    codesign --force --deep --sign "$IDENTITY" --identifier com.gladys.app \
      -r='designated => anchor apple generic and identifier "com.gladys.app"' "$DIST/gladys.app"
    ;;
  windows) "$QT_ROOT_DIR/bin/windeployqt" --release --qmldir src/qml "$DIST/gladys.exe" ;;
esac
