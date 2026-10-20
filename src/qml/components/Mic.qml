// Mic - Closed dot morphs into the open mic with live audio bars.

pragma ComponentBehavior: Bound

import QtQuick

Item {
    id: mic

    property bool expanded: false
    property real progress: expanded ? 1 : 0 // 0 = dot, 1 = mic open

    readonly property real scaleFactor: 0.5 + 0.5 * progress
    readonly property real micCenterY: height - 80 // mic center, above the dot
    // Average bar height normalized to the tallest bar (48 px, see audio_bars.cc).
    readonly property real audioLevel: {
        let sum = 0;
        for (const h of audioBars.heights)
            sum += h;
        return Math.min(sum / (audioBars.heights.length * 48), 1);
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
        x: (mic.width - width) / 2
        y: mic.micCenterY - height / 2
        color: "#33FFD94A"
        opacity: mic.progress
    }

    Repeater {
        model: audioBars.heights.length

        Rectangle {
            required property int index

            width: 5
            height: audioBars.heights[index] * mic.scaleFactor
            radius: 2.5
            x: index * 6
            y: mic.micCenterY - height / 2
            color: "#FFD94A"
            opacity: mic.progress
            visible: mic.progress > 0
        }
    }

    Image {
        source: "qrc:/icons/icon-app.svg"
        sourceSize: Qt.size(48, 48)
        width: 48 * mic.scaleFactor
        height: width
        x: (mic.width - width) / 2
        y: mic.micCenterY - height / 2
        smooth: true
        opacity: mic.progress
    }

    Rectangle {
        id: dot

        width: 15
        height: 15
        radius: width / 2
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        color: mic.expanded ? "#FF0000" : "#FFFFFF"
        border.color: mic.expanded ? "#FF0000" : "#7A7A7A"
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
