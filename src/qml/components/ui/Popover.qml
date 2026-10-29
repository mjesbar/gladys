// Popover - Floating panel under target. Fades and scales in.

import QtQuick
import QtQuick.Controls.Basic as Basic
import ".."

Basic.Popup {
  id: pop

  property Item target
  property int gap: 8
  default property alias content: body.data

  parent: Overlay.overlay
  x: target ? target.mapToItem(parent, 0, 0).x : 0
  y: target ? target.mapToItem(parent, 0, target.height).y + gap : 0
  padding: Theme.padding

  background: Box {}

  contentItem: Column {
    id: body

    spacing: Theme.gap
  }

  enter: Transition {
    ParallelAnimation {
      NumberAnimation { property: "opacity"; from: 0; to: 1; duration: Theme.durationBase }
      NumberAnimation { property: "scale"; from: 0.96; to: 1; duration: Theme.durationBase }
    }
  }

  exit: Transition {
    NumberAnimation { property: "opacity"; to: 0; duration: Theme.durationFast }
  }
}
