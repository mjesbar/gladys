// HistoryLogs - History > Logs: last 30 days, newest first, 20 per page. Toggle saves on change.

pragma ComponentBehavior: Bound

import QtQuick
import ".."
import "../.."
import "../ui" as Ui

Section {
  id: logs

  title: qsTr("Logs")
  readonly property bool hovered: list.hovered

  function load(s) {
    storeSwitch.checked = s.storeLogs;
  }

  function reload() {
    list.reset();
  }

  function refresh() {
    list.refresh();
  }

  Note {
    text: qsTr("Only the last 30 days of logs are stored.")
  }
  Row {
    width: parent.width
    spacing: 12

    Ui.Text {
      width: parent.width - storeSwitch.width - parent.spacing
      height: storeSwitch.height
      text: qsTr("Store Logs")
      verticalAlignment: Text.AlignVCenter
    }
    Ui.Switch {
      id: storeSwitch

      onToggled: settingsBridge.apply({
        storeLogs: checked
      })
    }
  }

  PagedList {
    id: list

    fetch: (offset, limit) => historyBridge.logs(offset, limit)

    delegate: Column {
      id: entry

      required property string datetime
      required property string log
      required property bool is_err
      required property bool is_warn

      width: ListView.view.width - Theme.spacingMd  // rows stop at the scrollbar lane
      spacing: 4

      Note {
        text: entry.datetime + (entry.is_err ? qsTr(" · error") : entry.is_warn ? qsTr(" · warning") : "")
      }
      Ui.Selectable {
        width: parent.width
        text: entry.log
      }
    }
  }
}
