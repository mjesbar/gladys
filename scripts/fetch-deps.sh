#!/usr/bin/env bash
# Download llama.cpp binaries and the llama / sherpa-onnx models into $GLADYS_SPACE/dist/.
# Usage: scripts/fetch-deps.sh [x64|arm64]
set -euo pipefail

usage() { echo "Usage: scripts/fetch-deps.sh [x64|arm64]" >&2; exit 1; }
[ $# -le 1 ] || usage

case "$(uname -s)" in Darwin) DEFAULT_ARCH=arm64 ;; *) DEFAULT_ARCH=x64 ;; esac
ARCH=${1:-$DEFAULT_ARCH}
case "$ARCH" in x64|arm64) ;; *) usage ;; esac

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
[ -d "$SPACE/dist/gladys.app/Contents/MacOS/sherpaonnx" ] && DEPS="$SPACE/dist/gladys.app/Contents/MacOS"
LLAMACPP_DIR="$DEPS/llama.cpp"
SHERPAONNX_DIR="$DEPS/sherpaonnx"

case "$(uname -s)-$ARCH" in
  Linux-x64)    LLAMACPP_FILE=$LLAMACPP_VER-bin-ubuntu-vulkan-x64.tar.gz ;;
  Linux-arm64)  LLAMACPP_FILE=$LLAMACPP_VER-bin-ubuntu-vulkan-arm64.tar.gz ;;
  Darwin-arm64) LLAMACPP_FILE=$LLAMACPP_VER-bin-macos-arm64.tar.gz ;;
  Darwin-x64)   LLAMACPP_FILE=$LLAMACPP_VER-bin-macos-x64.tar.gz ;;
  *-x64)        LLAMACPP_FILE=$LLAMACPP_VER-bin-win-vulkan-x64.zip ;;
  *-arm64)      LLAMACPP_FILE=$LLAMACPP_VER-bin-win-cpu-arm64.zip ;;
esac

mkdir -p "$LLAMACPP_DIR" "$SHERPAONNX_DIR"

# llama.cpp binary archive + extraction (re-extract when the arch changes)
[ -f "$LLAMACPP_DIR/$LLAMACPP_FILE" ] ||
  curl -fL -o "$LLAMACPP_DIR/$LLAMACPP_FILE" "$LLAMACPP_BASE/$LLAMACPP_FILE"
if [ "$(cat "$LLAMACPP_DIR/$LLAMACPP_VER/.arch" 2>/dev/null)" != "$ARCH" ]; then
  rm -rf "$LLAMACPP_DIR/$LLAMACPP_VER"
  mkdir -p "$LLAMACPP_DIR/$LLAMACPP_VER"
  case "$LLAMACPP_FILE" in
    *.zip) unzip -o "$LLAMACPP_DIR/$LLAMACPP_FILE" -d "$LLAMACPP_DIR/$LLAMACPP_VER" ;;
    *)     tar xzf "$LLAMACPP_DIR/$LLAMACPP_FILE" -C "$LLAMACPP_DIR/$LLAMACPP_VER" --strip-components=1 ;;
  esac
  echo "$ARCH" > "$LLAMACPP_DIR/$LLAMACPP_VER/.arch"
fi

# llama.cpp GGUF model (created after extraction, so extraction is never skipped)
LLAMACPP_MODEL_PATH="$LLAMACPP_DIR/$LLAMACPP_VER/models/$LLAMACPP_MODEL"
if [ ! -f "$LLAMACPP_MODEL_PATH" ]; then
  mkdir -p "$(dirname "$LLAMACPP_MODEL_PATH")"
  curl -fL -o "$LLAMACPP_MODEL_PATH" "$LLAMACPP_MODEL_URL"
fi

# sherpa-onnx model archive + extraction
SHERPA_ARCHIVE="$SHERPAONNX_DIR/$SHERPAONNX_MODEL.tar.bz2"
[ -f "$SHERPA_ARCHIVE" ] || curl -fL -o "$SHERPA_ARCHIVE" "$SHERPAONNX_MODEL_URL"
[ -d "$SHERPAONNX_DIR/$SHERPAONNX_MODEL" ] || tar xjf "$SHERPA_ARCHIVE" -C "$SHERPAONNX_DIR"
