// HistoryTranscription - One stored transcription: datetime, text, and a button copying only the text.

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls.Basic
import ".."
import "../.."
import "../ui" as Ui

Rectangle {
  id: card

  required property string datetime
  required property string transcription

  property bool copied: false

  Timer {
    id: copiedTimer

    interval: 1500
    onTriggered: card.copied = false
  }

  implicitHeight: row.implicitHeight + 2 * Theme.spacingMd
  radius: Theme.radiusMd
  color: Theme.surface
  border.color: Theme.border

  // Hidden editor: the QML route to the clipboard, copying only the selected text.
  TextEdit {
    id: clipboard

    visible: false
  }

  Row {
    id: row

    x: Theme.spacingMd
    y: Theme.spacingMd
    width: card.width - 2 * Theme.spacingMd
    spacing: Theme.spacingMd

    Column {
      width: row.width - copyButton.width - row.spacing
      spacing: Theme.spacingXs

      Note {
        text: card.datetime
      }
      Ui.Selectable {
        width: parent.width
        text: card.transcription
      }
    }
    Ui.Button {
      id: copyButton

      variant: "icon"
      iconSource: "qrc:/icons/copy.svg"
      ToolTip.visible: hovered || card.copied
      ToolTip.text: card.copied ? qsTr("Copied to clipboard") : qsTr("Copy")
      onClicked: {
        clipboard.text = card.transcription;
        clipboard.selectAll();
        clipboard.copy();
        card.copied = true;
        copiedTimer.restart();
      }
    }
  }
}
