// SoundEnableSoundEffects - Sound > Enable Sound Effects: plays the sfx on overlay toggle, saved on change.

pragma ComponentBehavior: Bound

import ".."
import "../ui" as Ui

Section {
  title: qsTr("Enable Sound Effects")

  function load(s) {
    effectsSwitch.checked = s.soundEffects;
  }

  Ui.Switch {
    id: effectsSwitch

    onToggled: settingsBridge.apply({
      soundEffects: checked
    })
  }
}
