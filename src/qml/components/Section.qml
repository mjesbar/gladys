// Section - Titled block inside a menu, with a rule under it unless it is the last one of its page.

pragma ComponentBehavior: Bound

import QtQuick

Column {
  id: sub

  default property alias content: body.data
  property string title
  property bool divided: true

  width: parent.width
  spacing: 12

  Text {
    text: sub.title
    color: Theme.text
    font.family: Theme.fontFamily
    font.pixelSize: 15
    font.bold: true
  }

  Column {
    id: body

    width: parent.width
    spacing: 12
  }

  Rectangle {
    visible: sub.divided
    width: parent.width
    height: 1
    color: Theme.border
  }
}
