// HistoryStatus - History > Status: health over the last 30 days of logs.

pragma ComponentBehavior: Bound

import ".."
import "../ui" as Ui

Section {
  title: qsTr("Status")

  property int health: 100

  function reload() {
    health = historyBridge.health();
  }

  Ui.Text {
    text: qsTr("Health: %1%").arg(health)
  }
  Note {
    text: qsTr("Share of error logs in the last 30 days. Warnings do not count.")
  }
}
