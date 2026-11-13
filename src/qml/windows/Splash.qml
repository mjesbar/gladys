// Splash - Centered logo with a spinner while the STT model loads.

import QtQuick
import QtQuick.Window

Window {
    id: splash

    signal fadeInFinished
    signal fadeOutFinished

    width: 440
    height: 394
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
        onFinished: splash.fadeInFinished()
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
        smooth: true
    }

    Image {
        x: (parent.width - width) / 2
        y: 120
        source: "qrc:/icons/title.svg"
        smooth: true
    }

    Image {
        x: (parent.width - width) / 2
        y: 358
        width: 24
        height: 24
        source: "qrc:/icons/spinner.svg"
        sourceSize: Qt.size(24, 24)
        smooth: true

        NumberAnimation on rotation {
            from: 0
            to: 360
            duration: 960
            loops: Animation.Infinite
        }
    }
}
