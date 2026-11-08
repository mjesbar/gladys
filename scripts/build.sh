#!/usr/bin/env bash
# Build Gladys (Release) in $GLADYS_SPACE, install it with its Qt/ICU runtime into $GLADYS_SPACE/dist/,
# fetch llama.cpp into dist/ and the default AI models into the user cache.
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


LLAMACPP_VER=llama-b9265
LLAMACPP_BASE=https://github.com/ggml-org/llama.cpp/releases/download/b9265
LLM_URL=https://huggingface.co/unsloth/Qwen3.5-0.8B-GGUF/resolve/main/Qwen3.5-0.8B-Q8_0.gguf
LLM_FILE=Qwen3.5-0.8B-Q8_0.gguf
LLM_SHA256=0ad885ffd4bb022fc4f0d33a3308fa108ef8613159d3b3a67e23abca056b7a6c
ASR_NAME=sherpa-onnx-nemo-parakeet-tdt-0.6b-v3-int8
ASR_URL=https://github.com/k2-fsa/sherpa-onnx/releases/download/asr-models/$ASR_NAME.tar.bz2
ASR_SHA256=1495e91b59d7fc53ddca91f3431758ec99f8fa253fdb0e1e851be0f72b14b0f9


# Same location as the app reads at runtime (src/modules/aimodmgr/aimodmgr.cc)
case "$OS" in
  linux) CACHE=$HOME/.cache/gladys/models ;;
  macos) CACHE=$HOME/Library/Caches/Gladys/Models ;;
  windows) CACHE="$(cygpath -u "$LOCALAPPDATA")/Gladys/Cache/Models" ;;
esac


sha256_of() {
  if command -v sha256sum >/dev/null; then sha256sum "$1" | cut -d' ' -f1; else shasum -a 256 "$1" | cut -d' ' -f1; fi
}


# fetch URL SHA256 DEST: downloads to DEST.part, checks the hash, then moves it to DEST. Skips when DEST exists.
fetch() {
  local url=$1 sha=$2 dest=$3
  if [ -e "$dest" ]; then return 0; fi
  mkdir -p "$(dirname "$dest")"
  echo "[build] download $(basename "$dest") ..."
  curl -fL -o "$dest.part" "$url"
  if [ "$(sha256_of "$dest.part")" != "$sha" ]; then
    rm -f "$dest.part"
    echo "[build] checksum mismatch: $(basename "$dest")" >&2
    exit 1
  fi
  mv "$dest.part" "$dest"
  echo "[build] download $(basename "$dest") OK"
}


# Bring deps back from the bundle (macOS, see below) so the clean step keeps them.
for loc in MacOS Resources; do
  [ -d "$DIST/gladys.app/Contents/$loc/llama.cpp" ] && mv "$DIST/gladys.app/Contents/$loc/llama.cpp" "$DIST/llama.cpp"
done


case "$OS-$ARCH" in
  linux-x64)     LLAMACPP_FILE=$LLAMACPP_VER-bin-ubuntu-vulkan-x64.tar.gz; LLAMACPP_SHA256=ad67b3e696679634637daafe0d0e27bbe1d26ecd94a6a8b57a79342918038730 ;;
  linux-arm64)   LLAMACPP_FILE=$LLAMACPP_VER-bin-ubuntu-vulkan-arm64.tar.gz; LLAMACPP_SHA256=0b92c537a4073c7b13dd8998401e87e5d08cd15ea4785e3d06b4928e1991fe9d ;;
  macos-arm64)   LLAMACPP_FILE=$LLAMACPP_VER-bin-macos-arm64.tar.gz; LLAMACPP_SHA256=cf4b70e091c9da3627362ff0cf68becae583e8bc97b9e4f3d2d9b86cd40095fb ;;
  macos-x64)     LLAMACPP_FILE=$LLAMACPP_VER-bin-macos-x64.tar.gz; LLAMACPP_SHA256=9095a5e6b286b7422e0907a6bfecf55d11265235841e3921f0de85c06efb217a ;;
  windows-x64)   LLAMACPP_FILE=$LLAMACPP_VER-bin-win-vulkan-x64.zip; LLAMACPP_SHA256=00e696e2c2507aa2876255e9f6f173468a5798bf0e6d25a3efb87d9aa914a710 ;;
  windows-arm64) LLAMACPP_FILE=$LLAMACPP_VER-bin-win-cpu-arm64.zip; LLAMACPP_SHA256=ef6a864d24ae17f936a9a3f806004c1653858371bd65401b0125cf0e992b94ab ;;
esac


# llama.cpp binaries, extracted into dist/llama.cpp/<ver>/ (re-extracted when the arch changes)
LLAMA_ARCHIVE="$DIST/llama.cpp/$LLAMACPP_FILE"
LLAMA_DIR="$DIST/llama.cpp/$LLAMACPP_VER"
fetch "$LLAMACPP_BASE/$LLAMACPP_FILE" "$LLAMACPP_SHA256" "$LLAMA_ARCHIVE"
if [ "$(cat "$LLAMA_DIR/.arch" 2>/dev/null)" != "$ARCH" ]; then
  echo "[build] llama.cpp $ARCH extract ..."
  rm -rf "$LLAMA_DIR"
  mkdir -p "$LLAMA_DIR"
  case "$LLAMACPP_FILE" in
    *.zip) unzip -o "$LLAMA_ARCHIVE" -d "$LLAMA_DIR" ;;
    *)     tar xzf "$LLAMA_ARCHIVE" -C "$LLAMA_DIR" --strip-components=1 ;;
  esac
  echo "$ARCH" > "$LLAMA_DIR/.arch"
  echo "[build] llama.cpp $ARCH extract OK"
fi


# Default models into the cache. The sherpa-onnx archive is extracted into a staging folder, then renamed.
fetch "$LLM_URL" "$LLM_SHA256" "$CACHE/llm/$LLM_FILE"
ASR_DIR="$CACHE/asr/$ASR_NAME"
if [ ! -d "$ASR_DIR" ]; then
  fetch "$ASR_URL" "$ASR_SHA256" "$ASR_DIR.tar.bz2"
  rm -rf "$ASR_DIR.extract"
  mkdir -p "$ASR_DIR.extract"
  tar -xjf "$ASR_DIR.tar.bz2" -C "$ASR_DIR.extract" --strip-components=1
  mv "$ASR_DIR.extract" "$ASR_DIR"
  rm -f "$ASR_DIR.tar.bz2"
  echo "[build] sherpa-onnx $ASR_NAME extract OK"
fi


# Clean dist/ but keep the downloaded llama.cpp folder.
echo "[build] clean dist ..."
[ -d "$DIST" ] && find "$DIST" -mindepth 1 -maxdepth 1 ! -name llama.cpp -exec rm -rf {} +
echo "[build] clean dist OK"


echo "[build] cmake configure ($OS $ARCH) ..."
cmake --preset release -B "$BUILD" -DGLADYS_ARCH="$ARCH" -DGLADYS_MA_DEBUG="$MA_DEBUG"
echo "[build] cmake configure OK"


# Copy the compile database to the project root so editors (Zed/clangd) find it.
if [ "$OS" = linux ]; then
  sudo cp "$BUILD/compile_commands.json" compile_commands.json
fi


echo "[build] cmake build ..."
cmake --build "$BUILD" --config Release
echo "[build] cmake build OK"


echo "[build] cmake install ..."
cmake --install "$BUILD" --prefix "$DIST"
echo "[build] cmake install OK"


# Gladys only uses SQLite. Other SQL drivers (mysql, psql, odbc...) need client libs the deploy tools cannot find.
# Qt's CMake config imports every driver file, so they are moved aside only for the bundle step and restored on exit.
SQL_ASIDE=$(mktemp -d)
find "$QT_ROOT_DIR/plugins/sqldrivers" -maxdepth 1 -type f ! -name '*qsqlite*' -exec mv {} "$SQL_ASIDE" \;
trap 'for f in "$SQL_ASIDE"/*; do [ -e "$f" ] && mv "$f" "$QT_ROOT_DIR/plugins/sqldrivers/"; done; rmdir "$SQL_ASIDE"' EXIT


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
      "$QT_ROOT_DIR/bin/linuxdeploy-x86_64.AppImage" --appdir "$DIST" \
      --executable "$DIST/gladys" --executable "$DIST/gladys-asr" \
      --plugin qt
    # Flatten usr/ back to dist/ root: the binary locates llama.cpp next to itself
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
    if [ -d "$DIST/llama.cpp" ]; then mv "$DIST/llama.cpp" "$DIST/gladys.app/Contents/MacOS/"; fi
    ;;


  windows)
    echo "[build] windeployqt bundle Qt ..."
    for exe in gladys gladys-asr; do
      "$QT_ROOT_DIR/bin/windeployqt" --release --qmldir src/qml "$DIST/$exe.exe"
    done
    echo "[build] windeployqt bundle Qt OK"
    ;;

esac
