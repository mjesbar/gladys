// KeyboardPushToTalkToggle - Keyboard > Push to talk / Toggle: Toggle flips recording, Hold records while the hotkey is down. Saved on change.

pragma ComponentBehavior: Bound

import ".."
import "../ui" as Ui

Section {
  title: qsTr("Push to talk / Toggle")

  function load(s) {
    modeCombo.currentIndex = Math.max(0, modeCombo.find(s.hotkeyMode));
  }

  Ui.Select {
    id: modeCombo

    width: parent.width
    model: settingsBridge.hotkeyModes
    onActivated: settingsBridge.apply({
      hotkeyMode: currentText
    })
  }
}
