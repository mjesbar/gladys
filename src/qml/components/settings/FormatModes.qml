// FormatModes - Format > Modes: output style and markdown mode, saved on Apply.

pragma ComponentBehavior: Bound

import QtQuick
import ".."
import "../ui" as Ui

Section {
  title: qsTr("Modes")
  divided: false

  function load(s) {
    markdownCombo.currentIndex = s.markdown ? 0 : 1;
    styleCombo.currentIndex = Math.max(0, styleCombo.find(s.style));
  }

  function values() {
    return {
      style: styleCombo.currentText,
      markdown: markdownCombo.currentIndex === 0
    };
  }

  Row {
    id: styleRow

    width: parent.width
    spacing: 12

    Ui.Text {
      id: styleLabel

      width: 80
      height: 40
      text: qsTr("Style")
      verticalAlignment: Text.AlignVCenter
    }
    Ui.Select {
      id: styleCombo

      width: styleRow.width - styleLabel.width - styleRow.spacing
      model: settingsBridge.styles
    }
  }
  Row {
    id: markdownRow

    width: parent.width
    spacing: 12

    Ui.Text {
      id: markdownLabel

      width: 80
      height: 40
      text: qsTr("Markdown")
      verticalAlignment: Text.AlignVCenter
    }
    Ui.Select {
      id: markdownCombo

      width: markdownRow.width - markdownLabel.width - markdownRow.spacing
      model: [qsTr("Yes"), qsTr("No")]
    }
  }
}
