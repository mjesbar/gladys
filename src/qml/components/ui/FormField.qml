// FormField - Label, control slot, hint and error stacked for one input.

import QtQuick
import "." as Ui
import ".."

Column {
  id: form

  property string label: ""
  property string hint: ""
  property string error: ""
  default property alias control: slot.data

  width: parent.width
  spacing: 6

  Ui.Text {
    text: form.label
    font.bold: true
    visible: form.label !== ""
  }
  Column {
    id: slot

    width: parent.width
    spacing: 6
  }
  Ui.Text {
    text: form.hint
    muted: true
    size: Theme.fontSize - 1
    visible: form.hint !== ""
  }
  Ui.Text {
    text: form.error
    color: Theme.danger
    size: Theme.fontSize - 1
    visible: form.error !== ""
    opacity: visible ? 1 : 0

    Behavior on opacity { NumberAnimation { duration: Theme.durationBase } }
  }
}
