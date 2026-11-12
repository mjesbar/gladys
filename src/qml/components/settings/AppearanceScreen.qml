// AppearanceScreen - Appearance > Screen: the screen the overlay is shown on, saved on change.

pragma ComponentBehavior: Bound

import ".."
import "../ui" as Ui

Section {
  title: qsTr("Screen")
  divided: false

  function load(s) {
    screenCombo.currentIndex = Math.max(0, screenCombo.find(s.screen));
  }

  Ui.Select {
    id: screenCombo

    width: parent.width
    model: settingsBridge.screens
    onActivated: settingsBridge.apply({
      screen: currentText
    })
  }
}
