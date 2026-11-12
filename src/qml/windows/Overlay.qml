// Overlay - Frameless window at the screen spot set by Position, hosting the profile of the Overlay Size and the Settings loader.

import QtQuick
import QtQuick.Window
import "../components"

Window {
    id: overlay

    property bool recording: false
    property bool processing: false
    property string placement: "up"
    property string overlaySize: "Minimal"
    readonly property bool expanded: recording || processing
    readonly property var profile: overlayBridge.profile(overlaySize, placement)
    readonly property var anchor: overlayBridge.dot_center(placement, Screen.width, Screen.height)

    width: profile.width
    height: profile.height
    x: Screen.virtualX + anchor.x - profile.dotX
    y: Screen.virtualY + anchor.y - profile.dotY
    color: "transparent"
    flags: (Qt.platform.os === "osx" ? Qt.Tool : Qt.ToolTip) | Qt.FramelessWindowHint | Qt.WindowStaysOnTopHint | Qt.NoDropShadowWindowHint | Qt.WindowTransparentForInput | (Qt.platform.os === "windows" ? Qt.WindowDoesNotAcceptFocus : 0)
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

    Component {
        id: minimalMic

        MicMinimal {
            dotX: overlay.profile.dotX
            dotY: overlay.profile.dotY
            recording: overlay.recording
            processing: overlay.processing
        }
    }

    Component {
        id: basicMic

        MicBasic {
            dotX: overlay.profile.dotX
            dotY: overlay.profile.dotY
            recording: overlay.recording
            processing: overlay.processing
        }
    }

    Component {
        id: greatMic

        MicGreat {
            dotX: overlay.profile.dotX
            dotY: overlay.profile.dotY
            expansion: overlay.profile.expansion
            expanded: overlay.expanded
            recording: overlay.recording
            processing: overlay.processing
        }
    }

    Loader {
        anchors.fill: parent
        sourceComponent: overlay.overlaySize === "Great" ? greatMic : overlay.overlaySize === "Basic" ? basicMic : minimalMic
    }

    Loader {
        id: settingsLoader
        active: false
        source: "Settings.qml"
    }
}
