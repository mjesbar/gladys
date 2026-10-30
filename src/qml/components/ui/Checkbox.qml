// Checkbox - Rounded square with a filled mark when checked.

import QtQuick
import "." as Ui
import QtQuick.Controls.Basic as Basic
import ".."

Basic.CheckBox {
  id: check

  spacing: 10
  topPadding: 4
  bottomPadding: 4

  indicator: Rectangle {
    x: check.leftPadding
    y: (check.height - height) / 2
    implicitWidth: 16
    implicitHeight: 16
    radius: 4
    color: check.checked ? Theme.accent : Theme.surface
    border.color: check.checked ? Theme.accent : Theme.border

    Behavior on color { ColorAnimation { duration: Theme.durationFast } }

    Rectangle {
      anchors.centerIn: parent
      width: 8
      height: 8
      radius: 2
      color: Theme.background
      opacity: check.checked ? 1 : 0

      Behavior on opacity { NumberAnimation { duration: Theme.durationFast } }
    }
  }

  contentItem: Ui.Text {
    text: check.text
    leftPadding: check.indicator.width + check.spacing
    verticalAlignment: Text.AlignVCenter
  }
}
