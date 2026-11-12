// DropdownMenu - Menu of string actions. Emits picked(value) when one is chosen.

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls.Basic as Basic
import "." as Ui
import "../.."

Basic.Menu {
  id: menu

  property var model: []

  signal picked(string value)

  background: Rectangle {
    implicitWidth: 180
    radius: Theme.radiusMd
    color: Theme.surface
    border.width: 0
  }

  // Slide: the menu unrolls downward from its top edge and rolls back up.
  readonly property int itemHeight: 40

  padding: 0
  clip: true
  enter: Transition {
    NumberAnimation { property: "height"; from: 0; to: menu.model.length * menu.itemHeight; duration: Theme.durationBase; easing.type: Easing.OutCubic }
  }

  exit: Transition {
    NumberAnimation { property: "height"; to: 0; duration: Theme.durationFast; easing.type: Easing.InCubic }
  }

  Repeater {
    model: menu.model

    delegate: Basic.MenuItem {
      id: option

      required property string modelData

      text: modelData
      height: menu.itemHeight
      onTriggered: menu.picked(modelData)

      HoverHandler {
        cursorShape: Qt.PointingHandCursor
      }

      contentItem: Ui.Text {
        text: option.text
        leftPadding: 12
        verticalAlignment: Text.AlignVCenter
      }
      background: Rectangle {
        color: Theme.hover  // fixed fill: only opacity animates, so no colour flicker
        opacity: option.highlighted || option.hovered ? 1 : 0

        Behavior on opacity { NumberAnimation { duration: Theme.durationBase } }
      }
    }
  }
}
