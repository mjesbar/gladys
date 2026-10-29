// Tabs - Tab bar over a stack. Each child is one page, titled by model[index].

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls.Basic as Basic
import "." as Ui
import ".."

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
    spacing: 8

    Repeater {
      model: tabs.model

      delegate: Basic.TabButton {
        id: tab

        required property string modelData

        padding: 8
        contentItem: Ui.Text {
          text: tab.modelData
          color: tab.checked ? Theme.text : Theme.muted
          font.bold: tab.checked
        }
        background: Rectangle {
          color: "transparent"

          Rectangle {
            anchors.bottom: parent.bottom
            width: parent.width
            height: 2
            color: Theme.accent
            opacity: tab.checked ? 1 : 0

            Behavior on opacity { NumberAnimation { duration: Theme.durationBase } }
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
