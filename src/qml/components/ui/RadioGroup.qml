// RadioGroup - One Radio per string in model; value holds the chosen string.

pragma ComponentBehavior: Bound

import QtQuick
import ".."

Column {
  id: group

  property var model: []
  property string value

  spacing: 8

  function mark(radio) {
    radio.checked = radio.modelData === group.value;
  }

  onValueChanged: {
    for (let i = 0; i < repeater.count; ++i)
      mark(repeater.itemAt(i));
  }

  Repeater {
    id: repeater

    model: group.model

    onItemAdded: (index, item) => group.mark(item)

    delegate: Radio {
      required property string modelData

      text: modelData
      onClicked: group.value = modelData
    }
  }
}
