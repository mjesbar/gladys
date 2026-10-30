// Tooltip - Small label shown after a hover delay. Fades in and out.

import QtQuick
import QtQuick.Controls.Basic as Basic
import "." as Ui
import ".."

Basic.ToolTip {
  id: tip

  delay: 500

  background: Rectangle {
    radius: Theme.radius
    color: Theme.surface
    border.color: Theme.border
  }

  contentItem: Ui.Text {
    text: tip.text
    size: Theme.fontSize - 1
  }

  enter: Transition {
    NumberAnimation { property: "opacity"; from: 0; to: 1; duration: Theme.durationFast }
  }

  exit: Transition {
    NumberAnimation { property: "opacity"; to: 0; duration: Theme.durationFast }
  }
}
