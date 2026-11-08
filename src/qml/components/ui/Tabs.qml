// Tabs - Full-width pill switch over a stack. Each child is one page, titled by model[index].

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls.Basic as Basic
import "." as Ui
import "../.."

Column {
  id: tabs

  property var model: []
  property alias currentIndex: bar.currentIndex
  default property alias pages: stack.data

  width: parent.width
  spacing: Theme.gap

  Basic.TabBar {
    id: bar

    width: tabs.width
    padding: Theme.spacingXs
    spacing: Theme.spacingXs

    background: Rectangle {
      radius: Theme.radiusPill
      color: Theme.surface
      border.color: Theme.border
    }

    Repeater {
      model: tabs.model

      delegate: Basic.TabButton {
        id: tab

        required property string modelData

        width: (bar.width - bar.leftPadding - bar.rightPadding - bar.spacing * (tabs.model.length - 1)) / tabs.model.length
        height: 36
        padding: 0
        contentItem: Ui.Text {
          width: tab.width
          text: tab.modelData
          color: tab.checked ? Theme.background : Theme.muted
          font.bold: tab.checked
          horizontalAlignment: Text.AlignHCenter
          verticalAlignment: Text.AlignVCenter

          Behavior on color { ColorAnimation { duration: Theme.durationBase } }
        }
        background: Rectangle {
          radius: Theme.radiusPill
          color: tab.checked ? Theme.accent : tab.hovered ? Theme.hover : "transparent"

          Behavior on color { ColorAnimation { duration: Theme.durationBase } }
        }
      }
    }
  }

  StackLayout {
    id: stack

    width: tabs.width
    currentIndex: bar.currentIndex
  }
}
