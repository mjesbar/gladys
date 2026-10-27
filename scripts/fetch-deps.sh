#!/usr/bin/env bash
# Download llama.cpp binaries and the llama / sherpa-onnx models into $GLADYS_SPACE/dist/.
# Usage: scripts/fetch-deps.sh [--os linux|macos|windows] [--arch x64|arm64]
set -euo pipefail

usage() {
  echo "Usage: scripts/fetch-deps.sh [--os linux|macos|windows] [--arch x64|arm64]" >&2;
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
while [ $# -gt 0 ]; do
  [ $# -ge 2 ] || usage
  case "$1" in
    --os) OS=$2 ;;
    --arch) ARCH=$2 ;;
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
[ "$OS" = "$HOST_OS" ] || { echo "Target $OS cannot be prepared on host $HOST_OS" >&2; exit 1; }

cd "$(dirname "$0")/.."

SPACE=${GLADYS_SPACE:-$HOME/gladys}
mkdir -p "$SPACE" && [ -w "$SPACE" ] || { echo "Build space not writable: $SPACE" >&2; exit 1; }

LLAMACPP_VER=llama-b9265
LLAMACPP_BASE=https://github.com/ggml-org/llama.cpp/releases/download/b9265
LLAMACPP_MODEL=Llama-3.2-1B-Instruct-Q4_K_M.gguf
LLAMACPP_MODEL_URL=https://huggingface.co/bartowski/Llama-3.2-1B-Instruct-GGUF/resolve/main/Llama-3.2-1B-Instruct-Q4_K_M.gguf?download=true
SHERPAONNX_MODEL=sherpa-onnx-nemo-parakeet-tdt-0.6b-v3-int8
SHERPAONNX_MODEL_URL=https://github.com/k2-fsa/sherpa-onnx/releases/download/asr-models/sherpa-onnx-nemo-parakeet-tdt-0.6b-v3-int8.tar.bz2

# After a macOS build the deps live inside the bundle: reuse them, never re-download.
DEPS="$SPACE/dist"
[ -d "$SPACE/dist/gladys.app/Contents/Resources/sherpaonnx" ] && DEPS="$SPACE/dist/gladys.app/Contents/Resources"

LLAMACPP_DIR="$DEPS/llama.cpp"
SHERPAONNX_DIR="$DEPS/sherpaonnx"

case "$OS-$ARCH" in
  linux-x64)     LLAMACPP_FILE=$LLAMACPP_VER-bin-ubuntu-vulkan-x64.tar.gz ;;
  linux-arm64)   LLAMACPP_FILE=$LLAMACPP_VER-bin-ubuntu-vulkan-arm64.tar.gz ;;
  macos-arm64)   LLAMACPP_FILE=$LLAMACPP_VER-bin-macos-arm64.tar.gz ;;
  macos-x64)     LLAMACPP_FILE=$LLAMACPP_VER-bin-macos-x64.tar.gz ;;
  windows-x64)   LLAMACPP_FILE=$LLAMACPP_VER-bin-win-vulkan-x64.zip ;;
  windows-arm64) LLAMACPP_FILE=$LLAMACPP_VER-bin-win-cpu-arm64.zip ;;
esac

mkdir -p "$LLAMACPP_DIR" "$SHERPAONNX_DIR"

# llama.cpp binary archive + extraction (re-extract when the arch changes)
if [ ! -f "$LLAMACPP_DIR/$LLAMACPP_FILE" ]; then
  echo "[deps] llama.cpp $LLAMACPP_FILE download ..."
  curl -fL -o "$LLAMACPP_DIR/$LLAMACPP_FILE" "$LLAMACPP_BASE/$LLAMACPP_FILE"
  echo "[deps] llama.cpp $LLAMACPP_FILE download OK"
fi

if [ "$(cat "$LLAMACPP_DIR/$LLAMACPP_VER/.arch" 2>/dev/null)" != "$ARCH" ]; then
  echo "[deps] llama.cpp $ARCH extract ..."
  rm -rf "$LLAMACPP_DIR/$LLAMACPP_VER"
  mkdir -p "$LLAMACPP_DIR/$LLAMACPP_VER"
  case "$LLAMACPP_FILE" in
    *.zip) unzip -o "$LLAMACPP_DIR/$LLAMACPP_FILE" -d "$LLAMACPP_DIR/$LLAMACPP_VER" ;;
    *)     tar xzf "$LLAMACPP_DIR/$LLAMACPP_FILE" -C "$LLAMACPP_DIR/$LLAMACPP_VER" --strip-components=1 ;;
  esac
  echo "$ARCH" > "$LLAMACPP_DIR/$LLAMACPP_VER/.arch"
  echo "[deps] llama.cpp $ARCH extract OK"
fi

# llama.cpp GGUF model (created after extraction, so extraction is never skipped)
LLAMACPP_MODEL_PATH="$LLAMACPP_DIR/$LLAMACPP_VER/models/$LLAMACPP_MODEL"
if [ ! -f "$LLAMACPP_MODEL_PATH" ]; then
  echo "[deps] llama.cpp model $LLAMACPP_MODEL download ..."
  mkdir -p "$(dirname "$LLAMACPP_MODEL_PATH")"
  curl -fL -o "$LLAMACPP_MODEL_PATH" "$LLAMACPP_MODEL_URL"
  echo "[deps] llama.cpp model $LLAMACPP_MODEL download OK"
fi

# sherpa-onnx model archive + extraction
SHERPA_ARCHIVE="$SHERPAONNX_DIR/$SHERPAONNX_MODEL.tar.bz2"
if [ ! -f "$SHERPA_ARCHIVE" ]; then
  echo "[deps] sherpa-onnx $SHERPAONNX_MODEL download ..."
  curl -fL -o "$SHERPA_ARCHIVE" "$SHERPAONNX_MODEL_URL"
  echo "[deps] sherpa-onnx $SHERPAONNX_MODEL download OK"
fi
if [ ! -d "$SHERPAONNX_DIR/$SHERPAONNX_MODEL" ]; then
  echo "[deps] sherpa-onnx $SHERPAONNX_MODEL extract ..."
  tar xjf "$SHERPA_ARCHIVE" -C "$SHERPAONNX_DIR"
  echo "[deps] sherpa-onnx $SHERPAONNX_MODEL extract OK"
fi
