// Radio - Round option with a dot that scales in when checked.

import QtQuick
import "." as Ui
import QtQuick.Controls.Basic as Basic
import "../.."

Basic.RadioButton {
  id: radio

  spacing: 10
  topPadding: 4
  bottomPadding: 4

  indicator: Rectangle {
    x: radio.leftPadding
    y: (radio.height - height) / 2
    implicitWidth: 16
    implicitHeight: 16
    radius: 8
    color: radio.hovered ? Theme.hover : Theme.surface
    border.color: radio.checked ? Theme.accent : radio.hovered ? Theme.muted : Theme.border

    Behavior on color { ColorAnimation { duration: Theme.durationBase } }
    Behavior on border.color { ColorAnimation { duration: Theme.durationBase } }

    Rectangle {
      anchors.centerIn: parent
      width: 8
      height: 8
      radius: 4
      color: Theme.accent
      scale: radio.checked ? 1 : 0

      Behavior on scale { NumberAnimation { duration: Theme.durationFast } }
    }
  }

  contentItem: Ui.Text {
    text: radio.text
    leftPadding: radio.indicator.width + radio.spacing
    verticalAlignment: Text.AlignVCenter
  }
}
