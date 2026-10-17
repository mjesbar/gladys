// Theme - Design tokens (grey scale, rounded, Roboto Slab) shared by all QML.

pragma Singleton

import QtQuick

QtObject {
    readonly property color background: "#000000"
    readonly property color surface: "#111111"
    readonly property color border: "#2A2A2A"
    readonly property color text: "#E6E6E6"
    readonly property color muted: "#7A7A7A"
    readonly property color hover: "#1C1C1C"
    readonly property color pressed: "#2A2A2A"
    readonly property color accent: "#BDBDBD"
    readonly property string fontFamily: "Roboto Slab"
    readonly property int fontSize: 13
}
