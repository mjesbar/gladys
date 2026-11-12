// Select - Rounded dropdown over a string model. Popup fades in; hover and focus tint the trigger.

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls.Basic as Basic
import "." as Ui
import "../.."

Basic.ComboBox {
  id: select

  readonly property int optionHeight: 40

  leftPadding: 16
  rightPadding: 40
  palette.mid: Theme.surface  // Basic draws the list edge with mid; match the popup fill

  HoverHandler {
    id: hover

    cursorShape: Qt.PointingHandCursor
  }

  indicator: Ui.Icon {
    x: select.width - width - 16
    y: (select.height - height) / 2
    source: "qrc:/icons/chevron-down.svg"
    size: 16
  }

  background: Rectangle {
    implicitHeight: 40
    radius: Theme.radiusPill
    color: hover.hovered ? Theme.hover : Theme.surface
    border.color: select.activeFocus ? Theme.focus : Theme.border

    Behavior on color { ColorAnimation { duration: Theme.durationBase } }
    Behavior on border.color { ColorAnimation { duration: Theme.durationBase } }
  }

  contentItem: Ui.Text {
    text: select.displayText
    verticalAlignment: Text.AlignVCenter
  }

  popup.y: select.height + 4  // gap between trigger and list
  popup.contentItem: ListView {
    clip: true
    implicitHeight: contentHeight
    model: select.popup.visible ? select.delegateModel : null
    currentIndex: select.highlightedIndex
  }
  popup.padding: 0
  popup.background: Rectangle {
    radius: Theme.radiusMd
    color: Theme.surface
    border.width: 0
  }

  // Slide: the list is revealed from its top edge, so it unrolls downward and rolls back up.
  popup.clip: true
  popup.enter: Transition {
    // Target from known sizes: the list is not laid out yet when the transition starts.
    NumberAnimation { property: "height"; from: 0; to: select.count * select.optionHeight; duration: Theme.durationBase; easing.type: Easing.OutCubic }
  }

  popup.exit: Transition {
    NumberAnimation { property: "height"; to: 0; duration: Theme.durationFast; easing.type: Easing.InCubic }
  }

  delegate: Basic.ItemDelegate {
    id: option

    required property int index
    required property string modelData

    width: select.width
    height: select.optionHeight
    highlighted: select.highlightedIndex === index

    HoverHandler {
      cursorShape: Qt.PointingHandCursor
    }
    contentItem: Ui.Text {
      text: option.modelData
      verticalAlignment: Text.AlignVCenter
    }
    background: Rectangle {
      // Edge rows take the popup's corner radius, so the highlight never overflows it.
      topLeftRadius: option.index === 0 ? Theme.radiusMd : 0
      topRightRadius: option.index === 0 ? Theme.radiusMd : 0
      bottomLeftRadius: option.index === select.count - 1 ? Theme.radiusMd : 0
      bottomRightRadius: option.index === select.count - 1 ? Theme.radiusMd : 0
      color: Theme.hover  // fixed fill: only opacity animates, so no colour flicker
      opacity: option.highlighted || option.hovered ? 1 : 0

      Behavior on opacity { NumberAnimation { duration: Theme.durationBase } }
    }
  }
}
