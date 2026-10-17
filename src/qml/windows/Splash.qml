// Splash - Centered logo with a spinner while the STT model loads.

import QtQuick
import QtQuick.Window

Window {
    id: splash

    signal fadeOutFinished()

    width: 120
    height: 160
    x: Screen.virtualX + (Screen.width - width) / 2
    y: Screen.virtualY + (Screen.height - height) / 2
    color: "transparent"
    flags: Qt.FramelessWindowHint | Qt.WindowStaysOnTopHint | Qt.Tool | Qt.NoDropShadowWindowHint
    visible: true

    function fadeOut() {
        fadeOutAnimation.start();
    }

    NumberAnimation on opacity {
        from: 0
        to: 1
        duration: 300
        easing.type: Easing.InOutQuad
    }

    NumberAnimation {
        id: fadeOutAnimation

        target: splash
        property: "opacity"
        to: 0
        duration: 300
        easing.type: Easing.InOutQuad
        onFinished: {
            splash.visible = false;
            splash.fadeOutFinished();
        }
    }

    Image {
        x: (parent.width - width) / 2
        source: "qrc:/icons/icon.svg"
        sourceSize: Qt.size(120, 120)
    }

    Image {
        x: (parent.width - width) / 2
        y: 136
        width: 24
        height: 24
        source: "qrc:/icons/spinner.svg"
        sourceSize: Qt.size(24, 24)

        NumberAnimation on rotation {
            from: 0
            to: 360
            duration: 960
            loops: Animation.Infinite
        }
    }
}
