// MicMinimal - Minimal profile: the grey dot, red while recording; spinner ring while processing.

pragma ComponentBehavior: Bound

import QtQuick
import "ui"

Item {
    id: mic

    property real dotX: width / 2
    property real dotY: height / 2
    property bool recording: false
    property bool processing: false

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
