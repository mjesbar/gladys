// AppearanceOverlaySize - Appearance > Overlay Size: one exclusive card per size, saved on change.

pragma ComponentBehavior: Bound

import QtQuick
import ".."
import "../ui" as Ui

Section {
  title: qsTr("Overlay Size")

  function load(s) {
    for (let i = 0; i < repeater.count; ++i) {
      const card = repeater.itemAt(i);
      card.checked = card.modelData === s.overlaySize;
    }
  }

  Row {
    width: parent.width
    spacing: Theme.spacingMd

    Repeater {
      id: repeater

      model: settingsBridge.overlaySizes

      delegate: Ui.RadioCard {
        required property string modelData

        width: (parent.width - parent.spacing * (repeater.count - 1)) / repeater.count
        text: modelData
        source: "qrc:/overlay/" + modelData.toLowerCase() + ".svg"
        onClicked: settingsBridge.apply({
          overlaySize: modelData
        })
      }
    }
  }
}
