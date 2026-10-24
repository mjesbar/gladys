#!/usr/bin/env bash
# Download llama.cpp binaries and the llama / sherpa-onnx models into $GLADYS_SPACE/dist/.
set -euo pipefail

cd "$(dirname "$0")/.."

SPACE=${GLADYS_SPACE:-$HOME/gladys}
mkdir -p "$SPACE" && [ -w "$SPACE" ] || { echo "Build space not writable: $SPACE" >&2; exit 1; }

LLAMACPP_VER=llama-b9265
LLAMACPP_BASE=https://github.com/ggml-org/llama.cpp/releases/download/b9265
LLAMACPP_MODEL=Llama-3.2-1B-Instruct-Q4_K_M.gguf
LLAMACPP_MODEL_URL=https://huggingface.co/bartowski/Llama-3.2-1B-Instruct-GGUF/resolve/main/Llama-3.2-1B-Instruct-Q4_K_M.gguf?download=true
SHERPAONNX_MODEL=sherpa-onnx-nemo-parakeet-tdt-0.6b-v3-int8
SHERPAONNX_MODEL_URL=https://github.com/k2-fsa/sherpa-onnx/releases/download/asr-models/sherpa-onnx-nemo-parakeet-tdt-0.6b-v3-int8.tar.bz2
LLAMACPP_DIR="$SPACE/dist/llama.cpp"
SHERPAONNX_DIR="$SPACE/dist/sherpaonnx"

case "$(uname -s)" in
  Linux)  LLAMACPP_FILE=llama-b9265-bin-ubuntu-vulkan-x64.tar.gz ;;
  Darwin) LLAMACPP_FILE=llama-b9265-bin-macos-arm64.tar.gz ;;
  *)      LLAMACPP_FILE=llama-b9265-bin-win-vulkan-x64.zip ;;
esac

mkdir -p "$LLAMACPP_DIR" "$SHERPAONNX_DIR"

# llama.cpp binary archive + extraction
[ -f "$LLAMACPP_DIR/$LLAMACPP_FILE" ] ||
  curl -fL -o "$LLAMACPP_DIR/$LLAMACPP_FILE" "$LLAMACPP_BASE/$LLAMACPP_FILE"
if [ ! -d "$LLAMACPP_DIR/$LLAMACPP_VER" ]; then
  mkdir -p "$LLAMACPP_DIR/$LLAMACPP_VER"
  case "$LLAMACPP_FILE" in
    *.zip) unzip -o "$LLAMACPP_DIR/$LLAMACPP_FILE" -d "$LLAMACPP_DIR/$LLAMACPP_VER" ;;
    *)     tar xzf "$LLAMACPP_DIR/$LLAMACPP_FILE" -C "$LLAMACPP_DIR/$LLAMACPP_VER" --strip-components=1 ;;
  esac
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
