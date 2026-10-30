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
    readonly property color danger: "#E5484D"
    readonly property color success: "#3FB950"
    readonly property color warning: "#D29922"
    readonly property color info: "#58A6FF"

    function toneColor(tone) {
        if (tone === "success") return success;
        if (tone === "warning") return warning;
        if (tone === "error") return danger;
        return info;
    }
    readonly property string fontFamily: "Roboto Slab"
    readonly property int fontSize: 13
    readonly property int radius: 12
    readonly property int radiusPill: 20
    readonly property int gap: 12
    readonly property int padding: 16
    readonly property int durationFast: 120
    readonly property int durationBase: 150
    readonly property int durationSlow: 200
}
