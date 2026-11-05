// MicBasic - Basic profile: the grey dot with three audio bars per side while recording; spinner while processing.

pragma ComponentBehavior: Bound

import QtQuick
import "ui"

Item {
    id: mic

    property real dotX: width / 2
    property real dotY: height / 2
    property bool recording: false
    property bool processing: false

    // Great's live bars, five per side: rank 0 sits nearest the dot and takes the Great bar nearest the row center.
    Repeater {
        model: 10

        Rectangle {
            required property int index

            readonly property int rank: index % 5
            readonly property int source: index < 5 ? 9 - rank : 10 + rank
            readonly property real centerX: mic.dotX + (index < 5 ? -1 : 1) * (14 + 6 * rank)

            width: 5
            height: waves.heights[source]
            radius: 2.5
            x: centerX - width / 2
            y: mic.dotY - height / 2
            color: "#FFD94A"
            border.color: "#B8860B"
            border.width: 1
            visible: mic.recording
        }
    }

    Rectangle {
        width: 15
        height: 15
        radius: width / 2
        x: mic.dotX - width / 2
        y: mic.dotY - height / 2
        color: mic.recording ? "#FF0000" : "#FFFFFF"
        border.color: mic.recording ? "#FF0000" : "#7A7A7A"
        border.width: 2

        Behavior on color { ColorAnimation { duration: 300 } }
        Behavior on border.color { ColorAnimation { duration: 300 } }
    }

    // Ring around the dot while the transcription is formatted and pasted.
    Spinner {
        size: 25
        x: mic.dotX - width / 2
        y: mic.dotY - height / 2
        opacity: mic.processing ? 1 : 0
        visible: opacity > 0

        Behavior on opacity { NumberAnimation { duration: 300 } }
    }
}
