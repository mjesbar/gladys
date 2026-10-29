// ComboBox - Rounded ComboBox with the app theme.

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls.Basic as Basic

Basic.ComboBox {
  id: combo

  leftPadding: 16
  rightPadding: 16

  background: Rectangle {
    implicitHeight: 40
    radius: 20
    color: Theme.surface
    border.color: combo.activeFocus ? Theme.accent : Theme.border
  }
  popup.background: Rectangle {
    radius: 12
    color: Theme.surface
    border.color: Theme.border
  }
  delegate: Basic.ItemDelegate {
    id: option

    required property int index
    required property string modelData

    width: combo.width
    highlighted: combo.highlightedIndex === index
    contentItem: Text {
      text: option.modelData
      color: Theme.text
      font: combo.font
      verticalAlignment: Text.AlignVCenter
    }
    background: Rectangle {
      radius: 12
      color: option.highlighted ? Theme.hover : "transparent"
    }
  }
}
