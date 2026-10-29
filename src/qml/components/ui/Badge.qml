// Badge - Small pill label in a tone (info, success, warning, error).

import QtQuick
import "." as Ui
import ".."

Rectangle {
  id: badge

  property string tone: "info"
  property string text: ""

  implicitWidth: label.implicitWidth + 16
  implicitHeight: 20
  radius: Theme.radiusPill
  color: Qt.alpha(Theme.toneColor(tone), 0.18)

  Ui.Text {
    id: label

    anchors.centerIn: parent
    text: badge.text
    size: Theme.fontSize - 2
    color: Theme.toneColor(badge.tone)
  }
}
