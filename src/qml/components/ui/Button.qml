// Button - Pill button: primary, ghost, or round icon-only (variant "icon").

pragma ComponentBehavior: Bound

import QtQuick
import "." as Ui
import QtQuick.Controls.Basic as Basic
import ".."

Basic.Button {
  id: button

  property string variant: "ghost"
  property url iconSource

  readonly property bool primary: variant === "primary"
  readonly property bool iconOnly: variant === "icon"

  implicitHeight: 40
  leftPadding: iconOnly ? 11 : 16
  rightPadding: leftPadding

  background: Rectangle {
    radius: Theme.radiusPill
    color: button.primary ? (button.hovered ? Theme.text : Theme.accent) : button.down ? Theme.pressed : button.hovered ? Theme.hover : Theme.surface
    border.color: button.primary ? Theme.accent : Theme.border
    scale: button.down ? 0.97 : 1

    Behavior on color { ColorAnimation { duration: Theme.durationBase } }
    Behavior on scale { NumberAnimation { duration: Theme.durationFast } }
  }

  contentItem: Item {
    implicitWidth: button.iconOnly ? 18 : label.implicitWidth
    implicitHeight: 18

    Icon {
      anchors.centerIn: parent
      visible: button.iconOnly
      source: button.iconSource
      size: 18
    }
    Ui.Text {
      id: label

      anchors.centerIn: parent
      visible: !button.iconOnly
      text: button.text
      color: button.primary ? Theme.background : Theme.text
      font.bold: button.primary
    }
  }
}
