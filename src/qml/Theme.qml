// Theme - Design system tokens: spacing, radius, Tailwind-based color scales, typography. Shared by all QML.

pragma Singleton

import QtQuick

QtObject {
    // Spacing
    readonly property int spacingXs: 4
    readonly property int spacingSm: 8
    readonly property int spacingMd: 12
    readonly property int spacingLg: 16
    readonly property int spacingXl: 24
    readonly property int spacingXxl: 32

    // Radius
    readonly property int radiusSm: 8
    readonly property int radiusMd: 12
    readonly property int radiusLg: 16
    readonly property int radiusPill: 20

    // Neutral (greys, Tailwind neutral)
    readonly property color neutral100: "#F5F5F5"
    readonly property color neutral200: "#E5E5E5"
    readonly property color neutral300: "#D4D4D4"
    readonly property color neutral400: "#A3A3A3"
    readonly property color neutral500: "#737373"
    readonly property color neutral600: "#525252"
    readonly property color neutral700: "#404040"
    readonly property color neutral800: "#262626"
    readonly property color neutral900: "#171717"

    // Primary (yellow, Tailwind amber)
    readonly property color amber100: "#FEF3C7"
    readonly property color amber200: "#FDE68A"
    readonly property color amber300: "#FCD34D"
    readonly property color amber400: "#FBBF24"
    readonly property color amber500: "#F59E0B"
    readonly property color amber600: "#D97706"
    readonly property color amber700: "#B45309"
    readonly property color amber800: "#92400E"
    readonly property color amber900: "#78350F"

    // Secondary (blue, Tailwind blue)
    readonly property color blue100: "#DBEAFE"
    readonly property color blue200: "#BFDBFE"
    readonly property color blue300: "#93C5FD"
    readonly property color blue400: "#60A5FA"
    readonly property color blue500: "#3B82F6"
    readonly property color blue600: "#2563EB"
    readonly property color blue700: "#1D4ED8"
    readonly property color blue800: "#1E40AF"
    readonly property color blue900: "#1E3A8A"

    // Semantic aliases over the scales
    readonly property color background: neutral800
    readonly property color sidebar: neutral700
    readonly property color surface: neutral700
    readonly property color border: neutral600
    readonly property color text: neutral100
    readonly property color muted: neutral400
    readonly property color hover: neutral600
    readonly property color pressed: neutral500
    readonly property color accent: amber400
    readonly property color focus: blue500

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

    // Typography
    readonly property string fontDisplay: "Roboto Slab"
    readonly property string fontSans: "sans-serif"
    readonly property int fontXs: 11
    readonly property int fontSm: 13
    readonly property int fontMd: 15
    readonly property int fontLg: 18
    readonly property int fontXl: 24

    // Aliases used by components
    readonly property int fontSize: fontSm
    readonly property int gap: spacingMd
    readonly property int padding: spacingLg

    readonly property int durationFast: 120
    readonly property int durationBase: 150
    readonly property int durationSlow: 200
}
