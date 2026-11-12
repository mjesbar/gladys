// Tabs - Fitted pill switch over a stack. A pill slides under the current tab; each child is one page.

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
  readonly property int tabWidth: 80
  readonly property int tabHeight: 32

  width: parent.width
  spacing: Theme.gap

  Basic.TabBar {
    id: bar

    width: tabs.tabWidth * tabs.model.length + spacing * (tabs.model.length - 1) + leftPadding + rightPadding
    height: tabs.tabHeight + topPadding + bottomPadding
    padding: Theme.spacingXs
    spacing: Theme.spacingXs

    background: Rectangle {
      radius: Theme.radiusPill
      color: Theme.surface
      border.color: Theme.border

      Rectangle {
        x: bar.leftPadding + bar.currentIndex * (tabs.tabWidth + bar.spacing)
        y: bar.topPadding
        width: tabs.tabWidth
        height: bar.height - bar.topPadding - bar.bottomPadding
        radius: Theme.radiusPill
        color: Theme.accent

        Behavior on x { NumberAnimation { duration: Theme.durationBase; easing.type: Easing.OutCubic } }
      }
    }

    Repeater {
      model: tabs.model

      delegate: Basic.TabButton {
        id: tab

        required property string modelData

        HoverHandler {
          cursorShape: Qt.PointingHandCursor
        }

        width: tabs.tabWidth
        height: tabs.tabHeight
        padding: 0
        background: Item {}
        contentItem: Item {
          implicitWidth: tab.width
          implicitHeight: tab.height

          Ui.Text {
            anchors.centerIn: parent
            text: tab.modelData
            color: tab.checked ? Theme.background : Theme.muted
            font.bold: tab.checked

            Behavior on color { ColorAnimation { duration: Theme.durationBase } }
          }
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
