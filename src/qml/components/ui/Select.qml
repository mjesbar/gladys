// Select - Rounded dropdown over a string model. Popup fades in.

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls.Basic as Basic
import "." as Ui
import ".."

Basic.ComboBox {
  id: select

  leftPadding: 16
  rightPadding: 16
  palette.mid: Theme.surface  // Basic draws the list edge with mid; match the popup fill

  background: Rectangle {
    implicitHeight: 40
    radius: Theme.radiusPill
    color: Theme.surface
    border.color: select.activeFocus ? Theme.accent : Theme.border
  }

  contentItem: Ui.Text {
    text: select.displayText
    verticalAlignment: Text.AlignVCenter
  }

  popup.background: Rectangle {
    radius: Theme.radius
    color: Theme.surface
    border.width: 0
  }

  popup.enter: Transition {
    NumberAnimation { property: "opacity"; from: 0; to: 1; duration: Theme.durationFast }
  }

  popup.exit: Transition {
    NumberAnimation { property: "opacity"; to: 0; duration: Theme.durationFast }
  }

  delegate: Basic.ItemDelegate {
    id: option

    required property int index
    required property string modelData

    width: select.width
    highlighted: select.highlightedIndex === index

    contentItem: Ui.Text {
      text: option.modelData
      verticalAlignment: Text.AlignVCenter
    }
    background: Rectangle {
      radius: Theme.radius  // same radius as the popup, so the highlight follows its corner
      color: option.highlighted ? Theme.hover : "transparent"
    }
  }
}
