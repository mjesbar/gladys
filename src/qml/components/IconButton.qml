// IconButton - Round 36px icon-only button (add and remove rows).

import QtQuick
import QtQuick.Controls.Basic

Button {
    id: button

    property url iconSource

    implicitWidth: 36
    implicitHeight: 36
    padding: 0

    background: Rectangle {
        radius: 18
        color: button.down ? Theme.pressed : button.hovered ? Theme.hover : Theme.surface
        border.color: Theme.border
    }

    contentItem: Image {
        x: (button.width - width) / 2
        y: (button.height - height) / 2
        width: 22
        height: 22
        source: button.iconSource
        sourceSize: Qt.size(44, 44)
    }
}
