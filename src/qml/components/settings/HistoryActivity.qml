// HistoryActivity - History > Activity: stored transcriptions, newest first, 20 per page. Toggle saves on change.

pragma ComponentBehavior: Bound

import QtQuick
import ".."
import "../ui" as Ui

Section {
  title: qsTr("Activity")

  function load(s) {
    storeSwitch.checked = s.storeTranscriptions;
  }

  function reload() {
    list.reset();
  }

  Row {
    width: parent.width
    spacing: 12

    Ui.Text {
      width: parent.width - storeSwitch.width - parent.spacing
      height: storeSwitch.height
      text: qsTr("Store Transcriptions")
      verticalAlignment: Text.AlignVCenter
    }
    Ui.Switch {
      id: storeSwitch

      onToggled: settingsBridge.apply({
        storeTranscriptions: checked
      })
    }
  }

  PagedList {
    id: list

    fetch: (offset, limit) => historyBridge.transcriptions(offset, limit)

    delegate: Column {
      id: entry

      required property string datetime
      required property string transcription

      width: ListView.view.width
      spacing: 4

      Note {
        text: entry.datetime
      }
      Ui.Text {
        width: parent.width
        text: entry.transcription
        wrapMode: Text.WordWrap
      }
    }
  }
}
