// Skeleton - Placeholder block that pulses while content loads.

import QtQuick
import ".."

Rectangle {
  implicitWidth: 120
  implicitHeight: 16
  radius: Theme.radius
  color: Theme.border

  SequentialAnimation on opacity {
    loops: Animation.Infinite

    NumberAnimation { to: 0.5; duration: 600 }
    NumberAnimation { to: 1; duration: 600 }
  }
}
