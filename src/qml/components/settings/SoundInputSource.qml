// SoundInputSource - Sound > Input Source: capture device, saved on change.

pragma ComponentBehavior: Bound

import ".."
import "../ui" as Ui

Section {
  title: qsTr("Input Source")

  function load(s) {
    const index = inputCombo.find(s.inputSource);
    inputCombo.currentIndex = index >= 0 ? index : 0;
  }

  Ui.Select {
    id: inputCombo

    width: parent.width
    model: settingsBridge.inputSources
    onActivated: settingsBridge.apply({
      inputSource: currentText
    })
  }
}
