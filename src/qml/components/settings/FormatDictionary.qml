// FormatDictionary - Format > Dictionary: rare words and their brief descriptions, saved on Apply.

pragma ComponentBehavior: Bound

import ".."

Section {
  title: qsTr("Dictionary")

  function load(s) {
    editor.load(s.dictionary);
  }

  function values() {
    return {
      dictionary: editor.pairs()
    };
  }

  MappingEditor {
    id: editor

    toHint: qsTr("Brief description")
  }
}
