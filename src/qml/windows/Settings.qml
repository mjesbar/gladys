// Settings - Main menus in the sidebar; each menu's sub-sections on the right. Real sections save; the rest are mocks. Lazy-loaded.

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Window
import QtQuick.Controls.Basic
import QtQuick.Layouts
import "../components"

Window {
  id: settings

  readonly property var menus: [qsTr("System"), qsTr("Sound"), qsTr("Appearance"), qsTr("Format"), qsTr("Keyboard"), qsTr("Meetings"), qsTr("History")]
  property int current: 1  // index in menus, Sound by default

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

  // One main menu: its sub-sections stacked vertically.
  component SettingsPage: Item {
    default property alias content: column.data

    implicitHeight: column.implicitHeight + 64

    Column {
      id: column

      x: 32
      y: 32
      width: parent.width - 64
      spacing: 24
    }
  }

  // Sub-section: heading, content, then a rule unless it is the last one of its page.
  component SubSection: Column {
    id: sub

    default property alias content: body.data
    property string title
    property bool divided: true

    width: parent.width
    spacing: 12

    Text {
      text: sub.title
      color: Theme.text
      font.family: Theme.fontFamily
      font.pixelSize: 15
      font.bold: true
    }

    Column {
      id: body

      width: parent.width
      spacing: 12
    }

    Rectangle {
      visible: sub.divided
      width: parent.width
      height: 1
      color: Theme.border
    }
  }

  component MockNote: Text {
    text: qsTr("Not implemented yet")
    color: Theme.text
    opacity: 0.6
    wrapMode: Text.WordWrap
    width: parent.width
  }

  component AliasEditor: Column {
    id: editor

    required property ListModel aliasModel
    property string toHint: qsTr("Replacement")

    width: parent.width
    spacing: 12

    function addAlias() {
      const from = fromField.text.trim();
      if (from === "") {
        return;
      }
      editor.aliasModel.append({
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
      model: editor.aliasModel

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
          onEditingFinished: editor.aliasModel.setProperty(row.index, "from", text.trim())
        }
        Field {
          width: (row.width - 52) / 2
          height: parent.height
          text: row.to
          onEditingFinished: editor.aliasModel.setProperty(row.index, "to", text.trim())
        }
        IconButton {
          y: (parent.height - height) / 2
          iconSource: "qrc:/icons/trash.svg"
          ToolTip.visible: hovered
          ToolTip.text: qsTr("Remove")
          onClicked: editor.aliasModel.remove(row.index)
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
        onAccepted: editor.addAlias()
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
        placeholderText: editor.toHint
        onAccepted: editor.addAlias()
      }
      IconButton {
        iconSource: "qrc:/icons/plus.svg"
        ToolTip.visible: hovered
        ToolTip.text: qsTr("Add Alias")
        onClicked: editor.addAlias()
      }
    }
  }

  // Sidebar: the main menus only.
  Column {
    id: sidebar

    anchors.left: parent.left
    anchors.top: parent.top
    x: 12
    y: 16
    width: 196
    spacing: 4

    Repeater {
      model: settings.menus

      delegate: ItemDelegate {
        id: menuItem

        required property int index
        required property string modelData

        width: sidebar.width
        text: modelData
        highlighted: settings.current === index
        onClicked: settings.current = index

        background: Rectangle {
          radius: 12
          color: menuItem.highlighted ? Theme.hover : "transparent"
        }
        contentItem: Text {
          text: menuItem.text
          color: Theme.text
          font: menuItem.font
          verticalAlignment: Text.AlignVCenter
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
        currentIndex: settings.current

        // 0: System (mocks)
        SettingsPage {
          SubSection {
            title: qsTr("Cloud / Local")
            MockNote {}
          }
          SubSection {
            title: qsTr("AI Models")
            MockNote {}
          }
          SubSection {
            title: qsTr("Store Transcriptions")
            divided: false
            MockNote {}
          }
        }

        // 1: Sound
        SettingsPage {
          SubSection {
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
          SubSection {
            title: qsTr("Enable Sound Effects")
            MockNote {}
          }
          SubSection {
            title: qsTr("Enable LLM")
            MockNote {}
          }
          SubSection {
            title: qsTr("Sound FXs")
            divided: false
            MockNote {}
          }
        }

        // 2: Appearance (mocks)
        SettingsPage {
          SubSection {
            title: qsTr("Theme")
            MockNote {}
          }
          SubSection {
            title: qsTr("Overlay Size")
            MockNote {}
          }
          SubSection {
            title: qsTr("Position")
            divided: false
            MockNote {}
          }
        }

        // 3: Format
        SettingsPage {
          SubSection {
            title: qsTr("Dictionary")
            AliasEditor {
              aliasModel: dictionaryModel
              toHint: qsTr("Brief description")
            }
          }
          SubSection {
            title: qsTr("Snippets")
            AliasEditor {
              aliasModel: snippetModel
            }
          }
          SubSection {
            title: qsTr("Modes")
            divided: false
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

        // 4: Keyboard (mocks)
        SettingsPage {
          SubSection {
            title: qsTr("Push to talk / Toggle")
            MockNote {}
          }
          SubSection {
            title: qsTr("Shortcuts")
            divided: false
            MockNote {}
          }
        }

        // 5: Meetings (mock)
        SettingsPage {
          SubSection {
            title: qsTr("Meetings")
            divided: false
            MockNote {
              text: qsTr("Coming soon!")
            }
          }
        }

        // 6: History (mocks)
        SettingsPage {
          SubSection {
            title: qsTr("Activity")
            MockNote {}
          }
          SubSection {
            title: qsTr("Status")
            MockNote {}
          }
          SubSection {
            title: qsTr("Errors")
            MockNote {}
          }
          SubSection {
            title: qsTr("Logs")
            divided: false
            MockNote {}
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
