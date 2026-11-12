// SoundFxs - Sound > Sound FXs: the toggle sfx, saved on change. Each row previews its sound.

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls.Basic as Basic
import ".."
import "../ui" as Ui

Section {
  title: qsTr("Sound FXs")

  // Stored names are lowercase; the list shows them capitalized.
  function label(name) {
    return name.charAt(0).toUpperCase() + name.slice(1);
  }

  function load(s) {
    const index = fxCombo.find(label(s.soundFx));
    fxCombo.currentIndex = index >= 0 ? index : 0;
  }

  Ui.Select {
    id: fxCombo

    width: parent.width
    model: settingsBridge.sfxs.map(label)
    onActivated: settingsBridge.apply({
      soundFx: currentText.toLowerCase()
    })

    // Row: the name selects, the round play button previews without selecting.
    delegate: Basic.ItemDelegate {
      id: option

      required property int index
      required property string modelData

      width: fxCombo.width
      height: fxCombo.optionHeight
      highlighted: fxCombo.highlightedIndex === index
      rightPadding: 0
      contentItem: Item {
        Ui.Text {
          anchors.left: parent.left
          anchors.verticalCenter: parent.verticalCenter
          text: option.modelData
        }
        Ui.Button {
          anchors.right: parent.right
          anchors.verticalCenter: parent.verticalCenter
          variant: "icon"
          iconSource: "qrc:/icons/play.svg"
          onClicked: sfx.preview(option.modelData.toLowerCase())
        }
      }
      background: Rectangle {
        topLeftRadius: option.index === 0 ? Theme.radiusMd : 0
        topRightRadius: option.index === 0 ? Theme.radiusMd : 0
        bottomLeftRadius: option.index === fxCombo.count - 1 ? Theme.radiusMd : 0
        bottomRightRadius: option.index === fxCombo.count - 1 ? Theme.radiusMd : 0
        color: Theme.hover
        opacity: option.highlighted || option.hovered ? 1 : 0

        Behavior on opacity { NumberAnimation { duration: Theme.durationBase } }
      }
      HoverHandler {
        cursorShape: Qt.PointingHandCursor
      }
    }
  }
}
