// Textarea - Multi-line field, sized to rows. invalid shows the danger border.

import QtQuick
import QtQuick.Controls.Basic as Basic
import ".."

Basic.TextArea {
  id: area

  property bool invalid: false
  property int rows: 3

  implicitHeight: rows * metrics.lineSpacing + topPadding + bottomPadding
  leftPadding: 12
  rightPadding: 12
  topPadding: 10
  bottomPadding: 10
  wrapMode: TextEdit.Wrap
  color: Theme.text
  placeholderTextColor: Theme.muted

  FontMetrics {
    id: metrics

    font: area.font
  }

  background: Rectangle {
    radius: Theme.radius
    color: Theme.surface
    border.color: area.invalid ? Theme.danger : area.activeFocus ? Theme.accent : Theme.border

    Behavior on border.color { ColorAnimation { duration: Theme.durationBase } }
  }
}
