// HistoryActivity - History > Activity: stored transcriptions, newest first, 20 per page. Toggle saves on change.

pragma ComponentBehavior: Bound

import QtQuick
import ".."
import "../.."
import "../ui" as Ui

Section {
  title: qsTr("Activity")
  readonly property bool hovered: list.hovered

  function load(s) {
    storeSwitch.checked = s.storeTranscriptions;
  }

  function reload() {
    list.reset();
  }

  function refresh() {
    list.refresh();
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

    delegate: HistoryTranscription {
      width: ListView.view.width - Theme.spacingMd  // rows stop at the scrollbar lane
    }
  }
}
