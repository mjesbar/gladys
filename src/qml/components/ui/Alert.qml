// Alert - Bordered message in a tone (info, success, warning, error). Fades in.

import QtQuick
import "." as Ui
import ".."

Box {
  id: alert

  property string tone: "info"
  property string text: ""

  border.color: Theme.toneColor(tone)

  NumberAnimation on opacity {
    from: 0
    to: 1
    duration: Theme.durationSlow
  }

  Ui.Text {
    width: parent.width
    text: alert.text
  }
}
