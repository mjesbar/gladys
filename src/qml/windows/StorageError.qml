// StorageError - Popup for a storage failure. One window per failed table; dictation keeps working.

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Window
import ".."
import "../components/ui" as Ui

Window {
  id: popup

  function show_error(what) {
    message.text = qsTr("Gladys could not save: %1").arg(what);
    visible = true;
  }

  width: 360
  height: 160
  title: qsTr("Gladys storage")
  color: Theme.background
  visible: false

  Column {
    anchors.fill: parent
    anchors.margins: Theme.padding
    spacing: Theme.gap

    Ui.Text {
      id: message

      width: parent.width
      wrapMode: Text.WordWrap
    }
    Ui.Button {
      anchors.right: parent.right
      text: qsTr("OK")
      onClicked: popup.close()
    }
  }
}
