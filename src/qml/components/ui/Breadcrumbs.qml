// Breadcrumbs - Trail of labels. Clicking an earlier label emits picked(index).

pragma ComponentBehavior: Bound

import QtQuick
import "." as Ui
import ".."

Row {
  id: trail

  property var model: []

  signal picked(int index)

  spacing: 8

  Repeater {
    model: trail.model

    delegate: Row {
      id: crumb

      required property int index
      required property string modelData

      readonly property bool last: index === trail.model.length - 1

      spacing: 8

      Ui.Text {
        text: crumb.modelData
        muted: !crumb.last

        MouseArea {
          anchors.fill: parent
          enabled: !crumb.last
          cursorShape: Qt.PointingHandCursor
          onClicked: trail.picked(crumb.index)
        }
      }
      Ui.Text {
        visible: !crumb.last
        text: "›"
        muted: true
      }
    }
  }
}
