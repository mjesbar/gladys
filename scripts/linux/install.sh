#!/usr/bin/env bash
# Install the latest Gladys release for the current user (no root): app in ~/.local/share/gladys, launcher in ~/.local/bin.
# Usage: install.sh [--uninstall]   (remote: curl -fsSL <raw-url> | bash -s -- [--uninstall])
set -euo pipefail


REPO=mjesbar/gladys
APP="$HOME/.local/share/gladys"
BIN="$HOME/.local/bin/gladys"
DESKTOP="$HOME/.local/share/applications/gladys.desktop"
CACHE="$HOME/.cache/gladys"


usage() {
  echo "Usage: install.sh [--uninstall]" >&2;
  exit 1;
}


MODE=install
case "${1:-}" in
  "") ;;
  --uninstall) MODE=uninstall ;;
  *) usage ;;
esac


if [ "$MODE" = uninstall ]; then
  echo "[gladys] uninstall ..."
  rm -rf "$APP" "$BIN" "$DESKTOP" "$CACHE"
  echo "[gladys] uninstall OK"
  exit 0
fi


[ "$(uname -s)-$(uname -m)" = Linux-x86_64 ] || { echo "Unsupported host: $(uname -sm)" >&2; exit 1; }


# releases/latest redirects to releases/tag/v<version>
TAG=$(curl -fsSLI -o /dev/null -w '%{url_effective}' "https://github.com/$REPO/releases/latest")
VERSION=${TAG##*/v}
NAME=gladys-linux-x64-$VERSION.tar.bz2
BASE="https://github.com/$REPO/releases/download/v$VERSION"

TMP=$(mktemp -d)
trap 'rm -rf "$TMP"' EXIT

echo "[gladys] download $NAME ..."
curl -fsSL -o "$TMP/$NAME" "$BASE/$NAME"
curl -fsSL -o "$TMP/$NAME.sha256" "$BASE/$NAME.sha256"
(cd "$TMP" && sha256sum -c "$NAME.sha256")
echo "[gladys] download $NAME OK"

# Extract next to the install path, then swap, so a failed run never leaves a half-installed app
mkdir -p "$(dirname "$APP")"
tar -xjf "$TMP/$NAME" -C "$TMP"
rm -rf "$APP.new"
mv "$TMP/gladys" "$APP.new"
rm -rf "$APP"
mv "$APP.new" "$APP"

mkdir -p "$(dirname "$BIN")" "$(dirname "$DESKTOP")"
ln -sfn "$APP/gladys" "$BIN"
cat > "$DESKTOP" <<EOF
[Desktop Entry]
Type=Application
Name=Gladys
Exec=$APP/gladys
Icon=$APP/gladys.svg
Terminal=false
Categories=Utility;
EOF

echo "[gladys] installed $VERSION in $APP"
case ":$PATH:" in
  *":$HOME/.local/bin:"*) ;;
  *) echo "[gladys] add ~/.local/bin to PATH to run 'gladys' from a shell" ;;
esac
