#!/usr/bin/env bash
# Build Gladys (Release) and install it with its Qt/ICU runtime into dist/.
# Requires QT_ROOT_DIR (default: local Qt 6.8.3).
set -euo pipefail

cd "$(dirname "$0")/.."

: "${QT_ROOT_DIR:=$PWD/qt-6.8.3/6.8.3/gcc_64}"
export QT_ROOT_DIR
[ -d "$QT_ROOT_DIR/lib" ] || { echo "QT_ROOT_DIR is not a Qt install: $QT_ROOT_DIR" >&2; exit 1; }

# Clean dist/ but keep the downloaded llama.cpp and sherpaonnx folders.
[ -d dist ] && find dist -mindepth 1 -maxdepth 1 ! -name llama.cpp ! -name sherpaonnx -exec rm -rf {} +

cmake --preset release
cmake --build --preset release
cmake --install build/ --prefix "$PWD/dist"
