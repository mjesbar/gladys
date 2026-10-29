// Toast - One auto-hiding message in a tone, bottom-right of its Portal. show() replaces it.

import QtQuick
import "." as Ui
import ".."

Box {
  id: toast

  property string tone: "info"
  property string message: ""
  property bool shown: false
  property int duration: 3000

  function show(text, newTone) {
    message = text;
    tone = newTone;
    shown = true;
    hideTimer.restart();
  }

  width: 320
  x: parent.width - width - Theme.padding
  y: parent.height - height - Theme.padding + (shown ? 0 : Theme.gap)
  opacity: shown ? 1 : 0
  border.color: Theme.toneColor(tone)

  Behavior on opacity { NumberAnimation { duration: Theme.durationSlow } }
  Behavior on y { NumberAnimation { duration: Theme.durationSlow } }

  Timer {
    id: hideTimer

    interval: toast.duration
    onTriggered: toast.shown = false
  }

  Ui.Text {
    width: parent.width
    text: toast.message
  }
}
