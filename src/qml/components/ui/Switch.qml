// Switch - Pill toggle with a knob that slides between states.

import QtQuick
import QtQuick.Controls.Basic as Basic
import ".."

Basic.Switch {
  id: toggle

  indicator: Rectangle {
    x: toggle.leftPadding
    y: (toggle.height - height) / 2
    implicitWidth: 40
    implicitHeight: 22
    radius: 11
    color: toggle.checked ? Theme.accent : Theme.surface
    border.color: Theme.border

    Behavior on color { ColorAnimation { duration: Theme.durationBase } }

    Rectangle {
      x: toggle.checked ? parent.width - width - 3 : 3
      y: 3
      width: 16
      height: 16
      radius: 8
      color: Theme.background

      Behavior on x { NumberAnimation { duration: Theme.durationBase } }
    }
  }
}
