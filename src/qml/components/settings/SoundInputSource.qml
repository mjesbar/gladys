// SoundInputSource - Sound > Input Source: capture device, saved on Apply.

pragma ComponentBehavior: Bound

import ".."
import "../ui" as Ui

Section {
  title: qsTr("Input Source")

  function load(s) {
    const index = inputCombo.find(s.inputSource);
    inputCombo.currentIndex = index >= 0 ? index : 0;
  }

  function values() {
    return {
      inputSource: inputCombo.currentText
    };
  }

  Ui.Select {
    id: inputCombo

    width: parent.width
    model: settingsBridge.inputSources
  }
}
