// RadioCard - Exclusive card choice: image on top, radio dot and label below. Siblings share the checked state.

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls.Basic as Basic
import "." as Ui
import "../.."

Basic.RadioButton {
  id: card

  property url source

  padding: Theme.spacingMd
  indicator: Item {}

  background: Rectangle {
    radius: Theme.radiusMd
    color: card.hovered ? Theme.hover : Theme.surface
    border.width: 2
    border.color: card.checked ? Theme.accent : Theme.border

    Behavior on color { ColorAnimation { duration: Theme.durationBase } }
    Behavior on border.color { ColorAnimation { duration: Theme.durationBase } }
  }

  contentItem: Column {
    spacing: Theme.spacingSm

    Image {
      width: 196
      height: 96
      source: card.source
      fillMode: Image.Pad
    }
    Row {
      spacing: Theme.spacingSm

      Rectangle {
        anchors.verticalCenter: parent.verticalCenter
        implicitWidth: 16
        implicitHeight: 16
        radius: 8
        color: Theme.surface
        border.color: card.checked ? Theme.accent : Theme.border

        Rectangle {
          anchors.centerIn: parent
          width: 8
          height: 8
          radius: 4
          color: Theme.accent
          scale: card.checked ? 1 : 0

          Behavior on scale { NumberAnimation { duration: Theme.durationFast } }
        }
      }
      Ui.Text {
        text: card.text
        verticalAlignment: Text.AlignVCenter
      }
    }
  }
}
