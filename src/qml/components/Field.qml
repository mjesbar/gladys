// Field - Fully rounded single-line text input used by the Settings window.

import QtQuick
import QtQuick.Controls.Basic

TextField {
    id: field

    implicitHeight: 40
    leftPadding: 12
    rightPadding: 12
    color: Theme.text
    placeholderTextColor: Theme.muted

    background: Rectangle {
        radius: 20
        color: Theme.surface
        border.color: field.activeFocus ? Theme.accent : Theme.border
    }
}
