// DropdownMenu - Menu of string actions. Emits picked(value) when one is chosen.

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls.Basic as Basic
import "." as Ui
import ".."

Basic.Menu {
  id: menu

  property var model: []

  signal picked(string value)

  background: Rectangle {
    implicitWidth: 180
    radius: Theme.radius
    color: Theme.surface
    border.color: Theme.border
  }

  enter: Transition {
    ParallelAnimation {
      NumberAnimation { property: "opacity"; from: 0; to: 1; duration: Theme.durationFast }
      NumberAnimation { property: "scale"; from: 0.96; to: 1; duration: Theme.durationFast }
    }
  }

  exit: Transition {
    NumberAnimation { property: "opacity"; to: 0; duration: Theme.durationFast }
  }

  Repeater {
    model: menu.model

    delegate: Basic.MenuItem {
      id: option

      required property string modelData

      text: modelData
      onTriggered: menu.picked(modelData)

      contentItem: Ui.Text {
        text: option.text
        leftPadding: 12
        verticalAlignment: Text.AlignVCenter
      }
      background: Rectangle {
        radius: Theme.radius
        color: option.highlighted ? Theme.hover : "transparent"
      }
    }
  }
}
