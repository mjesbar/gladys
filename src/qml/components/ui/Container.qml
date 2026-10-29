// Container - Centered block, capped at maxWidth.

import QtQuick

Item {
  property int maxWidth: 720

  width: Math.min(maxWidth, parent.width)
  anchors.horizontalCenter: parent.horizontalCenter
  implicitHeight: childrenRect.height
}
