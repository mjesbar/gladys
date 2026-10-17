// ThemedCheckBox - Checkbox with the Gladys round indicator.

import QtQuick
import QtQuick.Controls.Basic

CheckBox {
    id: check

    spacing: 10
    topPadding: 4
    bottomPadding: 4

    indicator: Rectangle {
        x: check.leftPadding
        y: (check.height - height) / 2
        implicitWidth: 16
        implicitHeight: 16
        radius: 8
        color: check.checked ? Theme.accent : Theme.surface
        border.color: check.checked ? Theme.accent : Theme.border
    }
}
