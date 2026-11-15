#!/bin/sh
# Install the latest Gladys release for the current user (no root): app in ~/Applications/gladys.app.
# Usage: install.sh [--uninstall]   (remote: curl -fsSL <raw-url> | sh -s -- [--uninstall])
set -eu


REPO=mjesbar/gladys
APP="$HOME/Applications/gladys.app"
CACHE="$HOME/Library/Caches/Gladys"


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
  printf '[gladys] keep AI models? (Yes/no) '
  read -r ANSWER 2>/dev/null < /dev/tty || ANSWER=
  rm -rf "$APP"
  case "$ANSWER" in
    [Nn]|[Nn][Oo]) rm -rf "$CACHE" ;;
    *) find "$CACHE" -mindepth 1 -maxdepth 1 ! -name Models -exec rm -rf {} + 2>/dev/null || true ;;
  esac
  echo "[gladys] uninstall OK"
  exit 0
fi


[ "$(uname -s)-$(uname -m)" = Darwin-arm64 ] || { echo "Unsupported host: $(uname -sm)" >&2; exit 1; }


NAME=gladys-macos-arm64.zip
BASE="https://github.com/$REPO/releases/latest/download"

TMP=$(mktemp -d)
trap 'rm -rf "$TMP"' EXIT

echo "[gladys] download $NAME ..."
curl -fsSL -o "$TMP/$NAME" "$BASE/$NAME"
curl -fsSL -o "$TMP/$NAME.sha256" "$BASE/$NAME.sha256"
(cd "$TMP" && shasum -a 256 -c "$NAME.sha256")
echo "[gladys] download $NAME OK"

# Extract next to the install path, then swap, so a failed run never leaves a half-installed app
mkdir -p "$(dirname "$APP")"
unzip -q "$TMP/$NAME" -d "$TMP/x"
rm -rf "$APP.new"
mv "$TMP/x/gladys.app" "$APP.new"
rm -rf "$APP"
mv "$APP.new" "$APP"

# Skip the Gatekeeper quarantine scan (no-op when the attribute is absent)
xattr -dr com.apple.quarantine "$APP" 2>/dev/null || true

echo "[gladys] installed in $APP"
