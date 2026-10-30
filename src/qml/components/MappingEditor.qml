// MappingEditor - Editable list of from -> to pairs, with a row to add new ones.

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls.Basic
import "ui" as Ui

Column {
  id: editor

  property string toHint: qsTr("Replacement")

  width: parent.width
  spacing: 12

  function load(pairs) {
    mappingModel.clear();
    for (const from of Object.keys(pairs)) {
      mappingModel.append({
        from: from,
        to: pairs[from]
      });
    }
  }

  function pairs() {
    const result = {};
    for (let i = 0; i < mappingModel.count; ++i) {
      const row = mappingModel.get(i);
      result[row.from] = row.to;
    }
    return result;
  }

  function addMapping() {
    const from = fromField.text.trim();
    if (from === "") {
      return;
    }
    mappingModel.append({
      from: from,
      to: toField.text.trim()
    });
    fromField.clear();
    toField.clear();
    fromField.forceActiveFocus();
  }

  ListModel {
    id: mappingModel
  }

  ListView {
    id: aliasView

    width: parent.width
    height: 180
    clip: true
    model: mappingModel

    delegate: Row {
      id: row

      required property int index
      required property string from
      required property string to

      width: aliasView.width
      height: 40
      spacing: 8

      Ui.Input {
        width: (row.width - 56) / 2
        height: parent.height
        text: row.from
        onEditingFinished: mappingModel.setProperty(row.index, "from", text.trim())
      }
      Ui.Input {
        width: (row.width - 56) / 2
        height: parent.height
        text: row.to
        onEditingFinished: mappingModel.setProperty(row.index, "to", text.trim())
      }
      Ui.Button {
        y: (parent.height - height) / 2
        variant: "icon"
        iconSource: "qrc:/icons/trash.svg"
        ToolTip.visible: hovered
        ToolTip.text: qsTr("Remove")
        onClicked: mappingModel.remove(row.index)
      }
    }
  }

  Row {
    id: addRow

    width: parent.width
    spacing: 8

    Ui.Input {
      id: fromField

      width: (addRow.width - 82) / 2
      placeholderText: qsTr("Original word")
      onAccepted: editor.addMapping()
    }
    Ui.Icon {
      source: "qrc:/icons/chevron-right.svg"
      size: 18
      y: (addRow.height - height) / 2
    }
    Ui.Input {
      id: toField

      width: (addRow.width - 82) / 2
      placeholderText: editor.toHint
      onAccepted: editor.addMapping()
    }
    Ui.Button {
      variant: "icon"
      iconSource: "qrc:/icons/plus.svg"
      ToolTip.visible: hovered
      ToolTip.text: qsTr("Add Alias")
      onClicked: editor.addMapping()
    }
  }
}
