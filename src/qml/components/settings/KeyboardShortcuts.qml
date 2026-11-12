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
    Qt.platform.os === "linux"
      ? qsTr("Warning: Fn does not work on Linux. Most keyboards handle Fn in firmware and never send it to the OS.")
      : qsTr("Warning: not compatible with all keyboards. Fn is often handled by the keyboard firmware and never reaches the OS."),
    qsTr("Warning: may cause glitches. AltGr is used for layout characters and can clash with typing.")
  ]

  function imageFor(profile) {
    if (profile === "Fn") {
      return "Fn.png";
    }
    if (profile === "AltGr") {
      return "AltGr.png";
    }
    return Qt.platform.os === "osx" ? "CtrlOptP.png" : "CtrlAltP.png";
  }

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

        // Stored value stays "Ctrl+Alt+P"; only the label and image show the macOS shortcut too.
        text: modelData === "Ctrl+Alt+P" ? qsTr("Ctrl+Alt+P / Ctrl+Opt+P") : modelData
        source: "qrc:/keyboard/" + imageFor(modelData)
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
