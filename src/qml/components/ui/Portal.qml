// Portal - Full-window layer on the window overlay, above content. Hosts toasts.

import QtQuick

Item {
  parent: Overlay.overlay
  anchors.fill: parent
  z: 1000
}
