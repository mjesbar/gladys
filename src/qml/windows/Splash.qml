// Splash - Centered logo with a spinner while the STT model loads.

import QtQuick
import QtQuick.Window

Window {
    id: splash

    signal fadeOutFinished()

    width: 260
    height: 240
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
        smooth: true
    }

    Item {
        id: title

        x: (parent.width - width) / 2
        y: 120
        width: label.implicitWidth
        height: label.implicitHeight

        Repeater {
            model: [[-3, 0], [3, 0], [0, -3], [0, 3], [-2, -2], [2, -2], [-2, 2], [2, 2]]

            Text {
                x: modelData[0]
                y: modelData[1]
                text: label.text
                font: label.font
                color: "#000000"
                renderType: Text.QtRendering
            }
        }

        Text {
            id: label

            text: "Gladys"
            font.family: "Molle"
            font.pixelSize: 72
            color: "#2F80ED"
            renderType: Text.QtRendering
        }
    }

    Image {
        x: (parent.width - width) / 2
        y: 204
        width: 24
        height: 24
        source: "qrc:/icons/spinner.svg"
        sourceSize: Qt.size(24, 24)
        smooth: true
        antialiasing: true

        NumberAnimation on rotation {
            from: 0
            to: 360
            duration: 960
            loops: Animation.Infinite
        }
    }
}
