// FormatSnippets - Format > Snippets: literal text and its replacement, saved on change.

pragma ComponentBehavior: Bound

import ".."

Section {
  title: qsTr("Snippets")

  function load(s) {
    editor.load(s.snippets);
  }

  MappingEditor {
    id: editor

    onEdited: settingsBridge.apply({
      snippets: editor.pairs()
    })
  }
}
