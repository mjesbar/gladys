// Card - Titled, bordered group used by the Settings window.

import QtQuick

Rectangle {
    id: card

    property string title: ""
    default property alias content: body.data

    color: Theme.surface
    border.color: Theme.border
    radius: 16
    implicitHeight: body.implicitHeight + 48

    Text {
        x: 20
        y: 12
        text: card.title
        color: Theme.muted
        font.family: Theme.fontFamily
        font.pixelSize: Theme.fontSize
    }

    Column {
        id: body
        x: 16
        y: 36
        width: parent.width - 32
        spacing: 10
    }
}
