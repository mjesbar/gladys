// FormatEnableLlm - Format > Enable LLM: off pastes the ASR text as is, saved on change.

pragma ComponentBehavior: Bound

import ".."
import "../ui" as Ui

Section {
  title: qsTr("Enable LLM")

  function load(s) {
    llmSwitch.checked = s.llm;
  }

  Ui.Switch {
    id: llmSwitch

    onToggled: settingsBridge.apply({
      llm: checked
    })
  }
}
