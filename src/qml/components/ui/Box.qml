// Box - Bordered surface that holds its content with padding.

import QtQuick
import ".."

Rectangle {
  id: box

  property int padding: Theme.padding
  default property alias content: body.data

  color: Theme.surface
  border.color: Theme.border
  radius: Theme.radius
  implicitHeight: body.childrenRect.height + 2 * padding

  Item {
    id: body

    anchors.fill: parent
    anchors.margins: box.padding
  }
}
