// AppearancePosition - Appearance > Position: overlay position on screen, saved on change.

pragma ComponentBehavior: Bound

import ".."
import "../ui" as Ui

Section {
  title: qsTr("Position")
  divided: false

  function load(s) {
    positionCombo.currentIndex = Math.max(0, positionCombo.find(s.position));
  }

  Ui.Select {
    id: positionCombo

    width: parent.width
    model: settingsBridge.positions
    onActivated: settingsBridge.apply({
      position: currentText
    })
  }
}
