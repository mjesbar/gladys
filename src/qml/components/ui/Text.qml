// Text - Body text with the app font. Muted for secondary copy.

import QtQuick as Q
import ".."

Q.Text {
  property bool muted: false
  property int size: Theme.fontSize

  color: muted ? Theme.muted : Theme.text
  font.family: Theme.fontFamily
  font.pixelSize: size
  wrapMode: Q.Text.WordWrap
}
