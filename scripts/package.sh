#!/usr/bin/env bash
# Package $GLADYS_SPACE/dist (built by scripts/build.sh) into $GLADYS_SPACE/release/ with a .sha256 per artifact. Models are not packaged.
# Usage: scripts/package.sh [--os linux|macos|windows] [--arch x64|arm64] --version <v>
set -euo pipefail


cd "$(dirname "$0")/.."


SPACE=${GLADYS_SPACE:-$HOME/gladys}
DIST="$SPACE/dist"
RELEASE="$SPACE/release"
STAGE=$(mktemp -d)
trap 'rm -rf "$STAGE"' EXIT


usage() {
  echo "Usage: scripts/package.sh [--os linux|macos|windows] [--arch x64|arm64] --version <v>" >&2;
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
VERSION=
while [ $# -gt 0 ]; do
  [ $# -ge 2 ] || usage
  case "$1" in
    --os) OS=$2 ;;
    --arch) ARCH=$2 ;;
    --version) VERSION=$2 ;;
    *) usage ;;
  esac
  shift 2
done


[ -n "$VERSION" ] || usage
case "$OS:$ARCH" in
  linux:x64|windows:x64|macos:arm64|macos:x64) ;;
  *) usage ;;
esac
[ "$OS" = "$HOST_OS" ] || { echo "Target $OS cannot be packaged on host $HOST_OS" >&2; exit 1; }


mkdir -p "$RELEASE"
# llama.cpp release archives stay out of the packages, the extracted binaries stay in
EXCLUDE=(-x '*.h' '*.hpp' '*.sh' '*miniaudio*' '*.zip' '*.tar.gz')
ARTIFACTS=()


# checksum FILE: writes FILE.sha256 next to the artifact (sha256sum format, verified by install.sh)
checksum() {
  local cmd="shasum -a 256"
  command -v sha256sum >/dev/null && cmd=sha256sum
  (cd "$RELEASE" && $cmd "$1" > "$1.sha256")
}


# ---- Archive dist/ ----
echo "[package] archive $OS $ARCH $VERSION ..."
case "$OS" in

  linux)
    NAME=gladys-linux-$ARCH-$VERSION
    # Install tree: dev files out, icon in (used by the .desktop launcher)
    cp -a "$DIST" "$STAGE/gladys"
    find "$STAGE/gladys" -type f \( -name '*.h' -o -name '*.hpp' -o -name '*.sh' -o -name '*miniaudio*' -o -name '*.zip' -o -name '*.tar.gz' \) -delete
    cp src/assets/icons/icon.svg "$STAGE/gladys/gladys.svg"
    tar -C "$STAGE" -cjf "$RELEASE/$NAME.tar.bz2" gladys
    ARTIFACTS+=("$NAME.tar.bz2")

    # AppImage: AppDir/usr/bin holds the install tree, so the RPATH ($ORIGIN/lib) still resolves
    APPDIR=$STAGE/AppDir
    mkdir -p "$APPDIR/usr"
    mv "$STAGE/gladys" "$APPDIR/usr/bin"
    cp "$APPDIR/usr/bin/gladys.svg" scripts/linux/gladys.desktop "$APPDIR/"
    cat > "$APPDIR/AppRun" <<'EOF'
#!/bin/sh
exec "$(dirname "$(readlink -f "$0")")/usr/bin/gladys" "$@"
EOF
    chmod +x "$APPDIR/AppRun"
    APPIMAGE_EXTRACT_AND_RUN=1 OUTPUT="$RELEASE/$NAME.AppImage" \
      "$QT_ROOT_DIR/bin/linuxdeploy-x86_64.AppImage" --appdir "$APPDIR" \
      --desktop-file "$APPDIR/gladys.desktop" --icon-file "$APPDIR/gladys.svg" --output appimage
    ARTIFACTS+=("$NAME.AppImage")
    ;;

  macos)
    NAME=gladys-macos-$ARCH-$VERSION
    (cd "$DIST" && zip -qry "$RELEASE/$NAME.zip" . "${EXCLUDE[@]}")
    ARTIFACTS+=("$NAME.zip")
    ;;

  windows)
    NAME=gladys-win-$ARCH-$VERSION
    (cd "$DIST" && zip -qr "$RELEASE/$NAME.zip" . "${EXCLUDE[@]}")
    ARTIFACTS+=("$NAME.zip")
    wix build -v "$(cygpath -w scripts/windows/gladys.wxs)" -arch x64 \
      -d Version="$VERSION" -d SourceDir="$(cygpath -w "$DIST")" \
      -o "$(cygpath -w "$RELEASE/$NAME.msi")"
    ARTIFACTS+=("$NAME.msi")
    ;;

esac
echo "[package] archive $OS $ARCH $VERSION OK"


echo "[package] checksum $OS $ARCH $VERSION ..."
for f in "${ARTIFACTS[@]}"; do checksum "$f"; done
echo "[package] checksum $OS $ARCH $VERSION OK"
