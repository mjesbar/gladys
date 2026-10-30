// Dialog - Modal popup with a title and body over a dimmed backdrop. Fades and scales in.

import QtQuick
import QtQuick.Controls.Basic
import ".."

Popup {
  id: dialog

  property string title: ""
  default property alias content: body.data

  signal accepted()
  signal rejected()

  function accept() {
    accepted();
    close();
  }

  function reject() {
    rejected();
    close();
  }

  parent: Overlay.overlay
  anchors.centerIn: parent
  implicitWidth: 360
  modal: true
  focus: true
  padding: Theme.padding

  background: Box {}

  contentItem: Column {
    id: body

    spacing: Theme.gap

    Heading {
      text: dialog.title
      visible: text !== ""
    }
  }

  Overlay.modal: Rectangle {
    color: "#B3000000"
    opacity: dialog.opacity
  }

  enter: Transition {
    ParallelAnimation {
      NumberAnimation { property: "opacity"; from: 0; to: 1; duration: Theme.durationBase }
      NumberAnimation { property: "scale"; from: 0.96; to: 1; duration: Theme.durationSlow }
    }
  }

  exit: Transition {
    NumberAnimation { property: "opacity"; to: 0; duration: Theme.durationFast }
  }
}
