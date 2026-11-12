// HistoryErrors - History > Errors: count of error logs in the last 30 days.

pragma ComponentBehavior: Bound

import ".."
import "../ui" as Ui

Section {
  title: qsTr("Errors")

  property int errorCount: 0

  function reload() {
    errorCount = historyBridge.error_count();
  }

  Ui.Text {
    text: qsTr("Errors in the last 30 days: %1").arg(errorCount)
  }
}
