// FormatSnippets - Format > Snippets: literal text and its replacement, saved on Apply.

pragma ComponentBehavior: Bound

import ".."

Section {
  title: qsTr("Snippets")

  function load(s) {
    editor.load(s.snippets);
  }

  function values() {
    return {
      snippets: editor.pairs()
    };
  }

  MappingEditor {
    id: editor
  }
}
