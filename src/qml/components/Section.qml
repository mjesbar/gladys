// Section - Titled block inside a menu, with a rule under it unless it is the last one of its page.

pragma ComponentBehavior: Bound

import QtQuick
import "ui" as Ui

Column {
  id: sub

  default property alias content: body.data
  property string title
  property bool divided: true

  width: parent.width
  spacing: 12

  Ui.Heading {
    level: 3
    text: sub.title
  }

  Column {
    id: body

    width: parent.width
    spacing: 12
  }

  Ui.Divider {
    visible: sub.divided
    width: parent.width
  }
}
