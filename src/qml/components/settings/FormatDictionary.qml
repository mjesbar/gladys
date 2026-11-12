// FormatDictionary - Format > Dictionary: rare words and their brief descriptions, saved on change.

pragma ComponentBehavior: Bound

import ".."

Section {
  title: qsTr("Dictionary")

  readonly property bool hovered: editor.hovered

  function load(s) {
    editor.load(s.dictionary);
  }

  MappingEditor {
    id: editor

    toHint: qsTr("Brief description")
    onEdited: settingsBridge.apply({
      dictionary: editor.pairs()
    })
  }
}
