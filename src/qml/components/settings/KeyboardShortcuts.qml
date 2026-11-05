// KeyboardShortcuts - Keyboard > Shortcuts: one exclusive hotkey profile per card, saved on change.

pragma ComponentBehavior: Bound

import QtQuick
import ".."
import "../ui" as Ui

Section {
  title: qsTr("Shortcuts")

  // Cards are 220 px wide (196 px image + card padding); each note sits under its card.
  readonly property var notes: [
    qsTr("Most reliable. Control+Alt+P on Windows/Linux, Control+Option+P on macOS."),
    qsTr("Warning: not compatible with all keyboards. Fn is often handled by the keyboard firmware and never reaches the OS."),
    qsTr("Warning: may cause glitches. AltGr is used for layout characters and can clash with typing.")
  ]

  function load(s) {
    for (let i = 0; i < cards.count; ++i) {
      const card = cards.itemAt(i);
      card.checked = card.modelData === s.hotkeyProfile;
    }
  }

  Row {
    id: cardRow

    width: parent.width
    spacing: Theme.spacingMd

    Repeater {
      id: cards

      model: settingsBridge.hotkeyProfiles

      delegate: Ui.RadioCard {
        required property string modelData

        // Stored value stays "Ctrl+Alt+P"; only the label shows the macOS shortcut too.
        text: modelData === "Ctrl+Alt+P" ? qsTr("Ctrl+Alt+P / Ctrl+Opt+P") : modelData
        source: "qrc:/keyboard/placeholder.svg"
        onClicked: settingsBridge.apply({
          hotkeyProfile: modelData
        })
      }
    }
  }

  Row {
    width: parent.width
    spacing: Theme.spacingMd

    Repeater {
      model: notes

      delegate: Note {
        required property string modelData

        width: 220
        text: modelData
      }
    }
  }
}
