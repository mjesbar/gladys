// MicGreat - Great profile: the dot on its border side opens toward the screen with live audio bars.

pragma ComponentBehavior: Bound

import QtQuick
import "ui"

Item {
    id: mic

    property real dotX: width / 2
    property real dotY: height - 7.5
    property string expansion: "down"  // side the mic opens toward: up | down | left | right
    property bool expanded: false
    property bool recording: false
    property bool processing: false
    property real progress: expanded ? 1 : 0 // 0 = dot, 1 = mic open

    readonly property real dirX: expansion === "right" ? 1 : expansion === "left" ? -1 : 0
    readonly property real dirY: expansion === "down" ? 1 : expansion === "up" ? -1 : 0
    readonly property real scaleFactor: 0.5 + 0.5 * progress
    readonly property real micSize: 48 * scaleFactor // diameter of the white circle
    readonly property real micCenterX: dotX + dirX * 72.5 // mic center, 72.5 px from the dot
    readonly property real micCenterY: dotY + dirY * 72.5
    // Average bar height normalized to the tallest bar (48 px, see waves.cc).
    readonly property real audioLevel: {
        let sum = 0;
        for (const h of waves.heights)
            sum += h;
        return Math.min(sum / (waves.heights.length * 48), 1);
    }

    Behavior on progress {
        NumberAnimation {
            duration: 300
            easing.type: Easing.InOutQuad
        }
    }

    // Halo behind the mic: flat translucent disc, no runtime blur. Pulses with audio.
    Rectangle {
        width: 48 + 24 * mic.audioLevel
        height: width
        radius: width / 2
        x: mic.micCenterX - width / 2
        y: mic.micCenterY - height / 2
        color: "#33FFD94A"
        opacity: mic.progress
    }

    // Vertical bars in a horizontal row, for every expansion.
    Repeater {
        model: waves.heights.length

        Rectangle {
            required property int index

            readonly property real slot: index * 6 - waves.heights.length * 3

            width: 5
            height: waves.heights[index] * mic.scaleFactor
            radius: 2.5
            x: mic.micCenterX + slot
            y: mic.micCenterY - height / 2
            color: "#FFD94A"
            border.color: "#B8860B"
            border.width: 1
            opacity: mic.progress
            visible: mic.progress > 0
        }
    }

    Image {
        source: "qrc:/icons/icon-app.svg"
        sourceSize: Qt.size(48, 48)
        width: mic.micSize
        height: width
        x: mic.micCenterX - width / 2
        y: mic.micCenterY - height / 2
        smooth: true
        opacity: mic.progress
    }

    // Drawn over the white circle while the transcription is formatted and pasted.
    Spinner {
        size: Math.round(mic.micSize)
        x: mic.micCenterX - width / 2
        y: mic.micCenterY - height / 2
        opacity: mic.processing ? 1 : 0
        visible: opacity > 0

        Behavior on opacity {
            NumberAnimation {
                duration: 300
                easing.type: Easing.InOutQuad
            }
        }
    }

    Rectangle {
        id: dot

        width: 15
        height: 15
        radius: width / 2
        x: mic.dotX - width / 2
        y: mic.dotY - height / 2
        color: mic.recording ? "#FF0000" : "#FFFFFF"
        border.color: mic.recording ? "#FF0000" : "#7A7A7A"
        border.width: 2

        Behavior on color {
            ColorAnimation {
                duration: 300
            }
        }
        Behavior on border.color {
            ColorAnimation {
                duration: 300
            }
        }
    }
}
