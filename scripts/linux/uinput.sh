#!/usr/bin/env bash
# Grant or revoke the current user access to /dev/uinput (Wayland paste).
# Usage: scripts/linux/uinput.sh {allow|deny}
set -euo pipefail

# Re-run with graphical root prompt
if [ "$EUID" -ne 0 ]; then
    exec pkexec "$(readlink -f "$0")" "$1"
fi
# Detect the actual user behind the root session
REAL_USER=$(logname)

allow() {
    echo 'KERNEL=="uinput", GROUP="input", MODE="0660", OPTIONS+="static_node=uinput", TAG+="uaccess"' > /etc/udev/rules.d/80-uinput.rules
    usermod -aG input "$REAL_USER"
    udevadm control --reload-rules && udevadm trigger
    echo "uinput" > /etc/modules-load.d/uinput.conf
    echo "Access granted to $REAL_USER. Please log out and back in."
}

deny() {
    rm -f /etc/udev/rules.d/80-uinput.rules
    rm -f /etc/modules-load.d/uinput.conf
    gpasswd -d "$REAL_USER" input
    udevadm control --reload-rules && udevadm trigger
    echo "Access denied for $REAL_USER."
}

case "$1" in
    allow) allow ;;
    deny)  deny ;;
    *)     echo "Usage: $0 {allow|deny}" >&2; exit 1 ;;
esac
