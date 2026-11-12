#!/usr/bin/env bash
# Install the latest Gladys release for the current user (no root): app in ~/Applications/gladys.app.
# Usage: install.sh [--uninstall]   (remote: curl -fsSL <raw-url> | bash -s -- [--uninstall])
set -euo pipefail


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
  rm -rf "$APP" "$CACHE"
  echo "[gladys] uninstall OK"
  exit 0
fi


[ "$(uname -s)-$(uname -m)" = Darwin-arm64 ] || { echo "Unsupported host: $(uname -sm)" >&2; exit 1; }


# releases/latest redirects to releases/tag/v<version>
TAG=$(curl -fsSLI -o /dev/null -w '%{url_effective}' "https://github.com/$REPO/releases/latest")
VERSION=${TAG##*/v}
NAME=gladys-macos-arm64-$VERSION.zip
BASE="https://github.com/$REPO/releases/download/v$VERSION"

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

echo "[gladys] installed $VERSION in $APP"
