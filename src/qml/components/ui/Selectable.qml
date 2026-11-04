// Selectable - Read-only body text the user can select and copy (mouse or Ctrl+C).

import QtQuick as Q
import "../.."

Q.TextEdit {
  readOnly: true
  selectByMouse: true
  color: Theme.text
  font.family: Theme.fontSans
  font.pixelSize: Theme.fontSize
  wrapMode: Q.TextEdit.WordWrap
}
