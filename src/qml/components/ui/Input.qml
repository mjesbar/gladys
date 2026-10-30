// Input - Rounded single-line field. invalid shows the danger border.

import QtQuick
import QtQuick.Controls.Basic as Basic
import ".."

Basic.TextField {
  id: field

  property bool invalid: false

  implicitHeight: 40
  leftPadding: 12
  rightPadding: 12
  color: Theme.text
  placeholderTextColor: Theme.muted

  background: Rectangle {
    radius: Theme.radiusPill
    color: Theme.surface
    border.color: field.invalid ? Theme.danger : field.activeFocus ? Theme.accent : Theme.border

    Behavior on border.color { ColorAnimation { duration: Theme.durationBase } }
  }
}
