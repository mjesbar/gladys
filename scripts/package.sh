#!/usr/bin/env bash
# Package Gladys with Qt runtime and models into release/.
# Usage: scripts/package.sh <linux|macos|windows> <arch> <version>
# Requires QT_ROOT_DIR pointing to the Qt install used to build.
set -euo pipefail

cd "$(dirname "$0")/.."

OS=${1:?usage: package.sh <linux|macos|windows> <arch> <version>}
ARCH=${2:?missing arch}
VERSION=${3:?missing version}
: "${QT_ROOT_DIR:?Set QT_ROOT_DIR to your Qt install dir}"

scripts/fetch-deps.sh

# ---- Qt runtime ----
copy_qt_plugins() {
  for plugin in platforms imageformats styles tls iconengines; do
    if [ -d "$QT_ROOT_DIR/plugins/$plugin" ]; then cp -r "$QT_ROOT_DIR/plugins/$plugin" dist/; fi
  done
}

case "$OS" in
  linux)
    find "$QT_ROOT_DIR/lib" -name '*.so*' -exec cp -P {} dist/ \;
    copy_qt_plugins
    ;;
  macos)
    for fw in Core Gui Widgets Network Svg; do
      cp -R "$QT_ROOT_DIR/lib/Qt$fw.framework" dist/
    done
    copy_qt_plugins
    ;;
  windows)
    cp "$QT_ROOT_DIR"/bin/*.dll dist/
    copy_qt_plugins
    ;;
  *) echo "Unknown OS: $OS" >&2; exit 1 ;;
esac

# ---- Clean dist/ for release ----
rm -rf dist/c-api
find dist -type f \( -name '*.tar.gz' -o -name '*.tar.bz2' -o -name '*.tar.xz' -o -name '*.zip' \) -delete
if [ "$OS" != linux ] && [ "$OS" != macos ]; then rm -f dist/gladys; fi
if [ "$OS" != windows ]; then rm -f dist/gladys.exe; fi

# ---- Stage and archive ----
STAGE=release/gladys-v$VERSION
mkdir -p "$STAGE"
rsync -a \
  --exclude=cuda \
  --exclude='*.h' \
  --exclude='*.hpp' \
  --exclude='*.sh' \
  --exclude=miniaudio \
  dist/ "$STAGE/"

case "$OS" in
  linux)   tar -C release -czf "release/gladys-linux-$ARCH-$VERSION.tar.gz" "gladys-v$VERSION" ;;
  macos)   (cd release && zip -r "gladys-macos-$ARCH-$VERSION.zip" "gladys-v$VERSION") ;;
  windows) (cd release && zip -r "gladys-win-$ARCH-$VERSION.zip" "gladys-v$VERSION") ;;
esac
