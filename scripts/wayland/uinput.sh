#!/bin/bash

# Ensure script re-runs with graphical root prompt
if [ "$EUID" -ne 0 ]; then
    pkexec "$0" "$1"
    exit $?
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
    *)     echo "Usage: $0 {allow|deny}" ;;
esac
