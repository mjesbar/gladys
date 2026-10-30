// Link - Text that opens href in the browser; underlines on hover.

import QtQuick
import "." as Ui
import ".."

Ui.Text {
  id: link

  property url href

  color: mouse.containsMouse ? Theme.text : Theme.accent
  font.underline: mouse.containsMouse

  Behavior on color { ColorAnimation { duration: Theme.durationBase } }

  MouseArea {
    id: mouse

    anchors.fill: parent
    hoverEnabled: true
    cursorShape: Qt.PointingHandCursor
    onClicked: Qt.openUrlExternally(link.href)
  }
}
