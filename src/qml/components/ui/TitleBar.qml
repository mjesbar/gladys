// TitleBar - Frameless window bar: macOS-style lights, centered title, drag to move.

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Window
import "." as Ui
import ".."

Item {
  id: bar

  property Window window
  property string title: ""

  height: 40

  component Light: Rectangle {
    id: light

    property string glyph: ""

    signal activated()

    width: 12
    height: 12
    radius: 6

    Ui.Text {
      anchors.centerIn: parent
      text: light.glyph
      size: 10
      color: "#000000"
      opacity: hover.hovered ? 1 : 0

      Behavior on opacity { NumberAnimation { duration: Theme.durationFast } }
    }
    HoverHandler {
      id: hover
    }
    MouseArea {
      anchors.fill: parent
      onClicked: light.activated()
    }
  }

  function toggleMaximized() {
    if (window.visibility === Window.Maximized)
      window.showNormal();
    else
      window.showMaximized();
  }

  MouseArea {
    anchors.fill: parent
    onPressed: bar.window.startSystemMove()
    onDoubleClicked: bar.toggleMaximized()
  }

  Ui.Text {
    anchors.centerIn: parent
    text: bar.title
    muted: true
  }

  Row {
    anchors.verticalCenter: parent.verticalCenter
    x: Theme.padding
    spacing: 8

    Light {
      color: "#FF5F57"
      glyph: "×"
      onActivated: bar.window.close()
    }
    Light {
      color: "#FEBC2E"
      glyph: "−"
      onActivated: bar.window.showMinimized()
    }
    Light {
      color: "#28C840"
      glyph: "+"
      onActivated: bar.toggleMaximized()
    }
  }
}
