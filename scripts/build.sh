#!/usr/bin/env bash
# Configure and build Gladys (Release) with CMakePresets.json.
# Requires QT_ROOT_DIR pointing to a Qt 6.8+ install.
set -euo pipefail

cd "$(dirname "$0")/.."

cmake --preset release
cmake --build --preset release
# ln -sfn build/compile_commands.json compile_commands.json
