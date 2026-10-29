// Settings - Sidebar of sections, one page per item. Real pages save; the rest are mocks. Lazy-loaded.

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Window
import QtQuick.Controls.Basic
import "../components"

Window {
  id: settings

  // Pages in the StackLayout: 0 input, 1 dictionary, 2 snippets, 3 modes, 4 placeholder.
  readonly property int placeholderPage: 4
  readonly property var sections: [
    {
      title: qsTr("System"),
      items: [{ label: qsTr("Cloud / Local"), page: 4 }, { label: qsTr("AI Models"), page: 4 }, { label: qsTr("Store Transcriptions"), page: 4 }]
    },
    {
      title: qsTr("Sound"),
      items: [{ label: qsTr("Input Source"), page: 0 }, { label: qsTr("Enable Sound Effects"), page: 4 }, { label: qsTr("Enable LLM"), page: 4 }, { label: qsTr("Sound FXs"), page: 4 }]
    },
    {
      title: qsTr("Appearance"),
      items: [{ label: qsTr("Theme"), page: 4 }, { label: qsTr("Overlay Size"), page: 4 }, { label: qsTr("Position"), page: 4 }]
    },
    {
      title: qsTr("Format"),
      items: [{ label: qsTr("Dictionary"), page: 1 }, { label: qsTr("Snippets"), page: 2 }, { label: qsTr("Modes"), page: 3 }]
    },
    {
      title: qsTr("Keyboard"),
      items: [{ label: qsTr("Push to talk / Toggle"), page: 4 }, { label: qsTr("Shortcuts"), page: 4 }]
    },
    {
      title: qsTr("Meetings"),
      items: [{ label: qsTr("Meetings"), page: 4, note: qsTr("Coming soon!") }]
    },
    {
      title: qsTr("History"),
      items: [{ label: qsTr("Activity"), page: 4 }, { label: qsTr("Status"), page: 4 }, { label: qsTr("Errors"), page: 4 }, { label: qsTr("Logs"), page: 4 }]
    }
  ]
  property var current: sections[1].items[0]

  width: 880
  height: 800
  minimumWidth: 640
  minimumHeight: 480
  title: qsTr("Gladys Settings")
  color: Theme.background
  transientParent: null
  visible: false

  palette.text: Theme.text
  palette.buttonText: Theme.text
  palette.windowText: Theme.text
  palette.base: Theme.surface
  palette.highlight: Theme.hover
  palette.highlightedText: Theme.text

  onVisibleChanged: if (visible)
    load()

  ListModel {
    id: dictionaryModel
  }

  ListModel {
    id: snippetModel
  }

  function fill(model, pairs) {
    model.clear();
    for (const from of Object.keys(pairs)) {
      model.append({
        from: from,
        to: pairs[from]
      });
    }
  }

  function collect(model) {
    const pairs = {};
    for (let i = 0; i < model.count; ++i) {
      const row = model.get(i);
      pairs[row.from] = row.to;
    }
    return pairs;
  }

  function load() {
    const s = settingsBridge.load();
    const index = inputCombo.find(s.inputSource);
    inputCombo.currentIndex = index >= 0 ? index : 0;
    fill(dictionaryModel, s.dictionary);
    fill(snippetModel, s.snippets);
    markdownCombo.currentIndex = s.markdown ? 0 : 1;
    styleCombo.currentIndex = Math.max(0, styleCombo.find(s.style));
  }

  function apply() {
    settingsBridge.apply({
      inputSource: inputCombo.currentText,
      style: styleCombo.currentText,
      markdown: markdownCombo.currentIndex === 0,
      dictionary: collect(dictionaryModel),
      snippets: collect(snippetModel)
    });
  }

  // Titled block that stacks its children vertically.
  component SettingsPage: Item {
    default property alias content: column.data

    implicitHeight: column.implicitHeight + 64

    Column {
      id: column

      x: 32
      y: 32
      width: parent.width - 64
      spacing: 16
    }
  }

  component AliasCard: Card {
    id: card

    required property ListModel aliasModel
    property string toHint: qsTr("Replacement")

    function addAlias() {
      const from = fromField.text.trim();
      if (from === "") {
        return;
      }
      card.aliasModel.append({
        from: from,
        to: toField.text.trim()
      });
      fromField.clear();
      toField.clear();
      fromField.forceActiveFocus();
    }

    ListView {
      id: aliasView

      width: parent.width
      height: 180
      clip: true
      model: card.aliasModel

      delegate: Row {
        id: row

        required property int index
        required property string from
        required property string to

        width: aliasView.width
        height: 40
        spacing: 8

        Field {
          width: (row.width - 52) / 2
          height: parent.height
          text: row.from
          onEditingFinished: card.aliasModel.setProperty(row.index, "from", text.trim())
        }
        Field {
          width: (row.width - 52) / 2
          height: parent.height
          text: row.to
          onEditingFinished: card.aliasModel.setProperty(row.index, "to", text.trim())
        }
        IconButton {
          y: (parent.height - height) / 2
          iconSource: "qrc:/icons/trash.svg"
          ToolTip.visible: hovered
          ToolTip.text: qsTr("Remove")
          onClicked: card.aliasModel.remove(row.index)
        }
      }
    }

    Row {
      id: addRow

      width: parent.width
      spacing: 8

      Field {
        id: fromField

        width: (addRow.width - 18 - 36 - 32) / 2
        placeholderText: qsTr("Original word")
        onAccepted: card.addAlias()
      }
      Item {
        width: 18
        height: 36
        Image {
          anchors.centerIn: parent
          source: "qrc:/icons/chevron-right.svg"
          sourceSize: Qt.size(18, 18)
        }
      }
      Field {
        id: toField

        width: (addRow.width - 18 - 36 - 32) / 2
        placeholderText: card.toHint
        onAccepted: card.addAlias()
      }
      IconButton {
        iconSource: "qrc:/icons/plus.svg"
        ToolTip.visible: hovered
        ToolTip.text: qsTr("Add Alias")
        onClicked: card.addAlias()
      }
    }
  }

  // Sidebar: sections with their items as a menu.
  Flickable {
    id: sidebar

    anchors.left: parent.left
    anchors.top: parent.top
    anchors.bottom: parent.bottom
    width: 220
    contentHeight: menu.height + 32
    clip: true

    ScrollBar.vertical: ScrollBar {}

    Column {
      id: menu

      x: 12
      y: 16
      width: sidebar.width - 24
      spacing: 4

      Repeater {
        model: settings.sections

        delegate: Column {
          id: section

          required property var modelData

          width: menu.width
          spacing: 2

          Text {
            x: 12
            topPadding: 12
            bottomPadding: 6
            text: section.modelData.title.toUpperCase()
            color: Theme.text
            opacity: 0.6
            font.pixelSize: 11
            font.bold: true
          }

          Repeater {
            model: section.modelData.items

            delegate: ItemDelegate {
              required property var modelData

              width: menu.width
              text: modelData.label
              highlighted: settings.current === modelData
              onClicked: settings.current = modelData

              background: Rectangle {
                radius: 12
                color: parent.highlighted ? Theme.hover : "transparent"
              }
              contentItem: Text {
                text: parent.text
                color: Theme.text
                font: parent.font
                verticalAlignment: Text.AlignVCenter
              }
            }
          }
        }
      }
    }
  }

  // Page area with the apply button pinned at the bottom.
  Item {
    anchors.left: sidebar.right
    anchors.right: parent.right
    anchors.top: parent.top
    anchors.bottom: parent.bottom

    Flickable {
      anchors.left: parent.left
      anchors.right: parent.right
      anchors.top: parent.top
      anchors.bottom: applyButton.top
      anchors.bottomMargin: 16
      contentHeight: stack.height
      clip: true

      ScrollBar.vertical: ScrollBar {}

      StackLayout {
        id: stack

        width: parent.width
        currentIndex: settings.current.page

        // 0: input source
        SettingsPage {
          Card {
            width: parent.width
            title: qsTr("Input Source")

            ComboBox {
              id: inputCombo

              width: parent.width
              leftPadding: 16
              rightPadding: 16
              model: settingsBridge.inputSources
              background: Rectangle {
                implicitHeight: 40
                radius: 20
                color: Theme.surface
                border.color: inputCombo.activeFocus ? Theme.accent : Theme.border
              }
              popup.background: Rectangle {
                radius: 12
                color: Theme.surface
                border.color: Theme.border
              }
              delegate: ItemDelegate {
                required property int index
                required property string modelData

                width: inputCombo.width
                highlighted: inputCombo.highlightedIndex === index
                contentItem: Text {
                  text: modelData
                  color: Theme.text
                  font: inputCombo.font
                  verticalAlignment: Text.AlignVCenter
                }
                background: Rectangle {
                  radius: 12
                  color: highlighted ? Theme.hover : "transparent"
                }
              }
            }
          }
        }

        // 1: dictionary
        SettingsPage {
          AliasCard {
            width: parent.width
            title: qsTr("Dictionary")
            toHint: qsTr("Brief description")
            aliasModel: dictionaryModel
          }
        }

        // 2: snippets
        SettingsPage {
          AliasCard {
            width: parent.width
            title: qsTr("Snippets")
            aliasModel: snippetModel
          }
        }

        // 3: modes (style and markdown)
        SettingsPage {
          Card {
            width: parent.width
            title: qsTr("Style")

            Row {
              id: styleRow

              width: parent.width
              spacing: 12

              Text {
                id: styleLabel

                width: 80
                height: 40
                text: qsTr("Style")
                color: Theme.text
                verticalAlignment: Text.AlignVCenter
              }
              ComboBox {
                id: styleCombo

                width: styleRow.width - styleLabel.width - styleRow.spacing
                leftPadding: 16
                rightPadding: 16
                model: settingsBridge.styles
                background: Rectangle {
                  implicitHeight: 40
                  radius: 20
                  color: Theme.surface
                  border.color: styleCombo.activeFocus ? Theme.accent : Theme.border
                }
                popup.background: Rectangle {
                  radius: 12
                  color: Theme.surface
                  border.color: Theme.border
                }
                delegate: ItemDelegate {
                  required property int index
                  required property string modelData

                  width: styleCombo.width
                  highlighted: styleCombo.highlightedIndex === index
                  contentItem: Text {
                    text: modelData
                    color: Theme.text
                    font: styleCombo.font
                    verticalAlignment: Text.AlignVCenter
                  }
                  background: Rectangle {
                    radius: 12
                    color: highlighted ? Theme.hover : "transparent"
                  }
                }
              }
            }
            Row {
              id: markdownRow

              width: parent.width
              spacing: 12

              Text {
                id: markdownLabel

                width: 80
                height: 40
                text: qsTr("Markdown")
                color: Theme.text
                verticalAlignment: Text.AlignVCenter
              }
              ComboBox {
                id: markdownCombo

                width: markdownRow.width - markdownLabel.width - markdownRow.spacing
                leftPadding: 16
                rightPadding: 16
                model: [qsTr("Yes"), qsTr("No")]
                background: Rectangle {
                  implicitHeight: 40
                  radius: 20
                  color: Theme.surface
                  border.color: markdownCombo.activeFocus ? Theme.accent : Theme.border
                }
                popup.background: Rectangle {
                  radius: 12
                  color: Theme.surface
                  border.color: Theme.border
                }
                delegate: ItemDelegate {
                  required property int index
                  required property string modelData

                  width: markdownCombo.width
                  highlighted: markdownCombo.highlightedIndex === index
                  contentItem: Text {
                    text: modelData
                    color: Theme.text
                    font: markdownCombo.font
                    verticalAlignment: Text.AlignVCenter
                  }
                  background: Rectangle {
                    radius: 12
                    color: highlighted ? Theme.hover : "transparent"
                  }
                }
              }
            }
          }
        }

        // 4: placeholder for items not implemented yet
        SettingsPage {
          Card {
            width: parent.width
            title: settings.current.label

            Text {
              width: parent.width
              text: settings.current.note ?? qsTr("Not implemented yet")
              color: Theme.text
              opacity: 0.6
              wrapMode: Text.WordWrap
            }
          }
        }
      }
    }

    Button {
      id: applyButton

      anchors.left: parent.left
      anchors.right: parent.right
      anchors.bottom: parent.bottom
      anchors.margins: 32
      text: qsTr("Apply")
      font.bold: true
      onClicked: settings.apply()

      background: Rectangle {
        implicitHeight: 40
        radius: 20
        color: applyButton.hovered ? Theme.text : Theme.accent
        border.color: Theme.accent
      }
      contentItem: Text {
        text: applyButton.text
        font: applyButton.font
        color: Theme.background
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
      }
    }
  }
}
