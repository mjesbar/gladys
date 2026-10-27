// Overlay - Frameless bottom-center window hosting the Mic and the Settings loader.

import QtQuick
import QtQuick.Window
import "../components"

Window {
    id: overlay

    readonly property int navBarPadding: 60 // keeps the mic clear of the OS nav bar
    property bool expanded: false

    width: 119
    height: 130
    color: "transparent"
    flags: (Qt.platform.os === "osx" ? Qt.Tool : Qt.ToolTip) | Qt.FramelessWindowHint | Qt.WindowStaysOnTopHint | Qt.NoDropShadowWindowHint | (Qt.platform.os === "windows" ? Qt.WindowDoesNotAcceptFocus : 0)
    // Centered on the screen the window is on (desktopAvailable* spans all screens).
    x: Screen.virtualX + (Screen.width - width) / 2
    y: Screen.desktopAvailableHeight - navBarPadding + 1 - height
    visible: true

    NumberAnimation on opacity {
        from: 0
        to: 1
        duration: 300
        easing.type: Easing.InOutQuad
    }

    function openSettings() {
        settingsLoader.active = true;
        settingsLoader.item.show();
        settingsLoader.item.raise();
        settingsLoader.item.requestActivate();
    }

    Mic {
        anchors.fill: parent
        expanded: overlay.expanded
    }

    Loader {
        id: settingsLoader
        active: false
        source: "Settings.qml"
    }
}
