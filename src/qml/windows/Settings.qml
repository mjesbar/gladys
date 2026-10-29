// Settings - Main menus in the sidebar; each menu's sub-sections on the right. Lazy-loaded.

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Window
import QtQuick.Controls.Basic
import QtQuick.Layouts
import "../components"
import "../components/settings"

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

  function load() {
    const s = settingsBridge.load();
    soundInputSource.load(s);
    formatDictionary.load(s);
    formatSnippets.load(s);
    formatModes.load(s);
  }

  function apply() {
    settingsBridge.apply(Object.assign({}, soundInputSource.values(), formatDictionary.values(), formatSnippets.values(), formatModes.values()));
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

  // Page area with the apply button pinned at the bottom. Flickable margins give each page its padding.
  Item {
    anchors.left: sidebar.right
    anchors.right: parent.right
    anchors.top: parent.top
    anchors.bottom: parent.bottom

    Flickable {
      id: flick

      anchors.left: parent.left
      anchors.right: parent.right
      anchors.top: parent.top
      anchors.bottom: applyButton.top
      anchors.bottomMargin: 16
      leftMargin: 32
      rightMargin: 32
      topMargin: 32
      bottomMargin: 32
      contentWidth: stack.width
      contentHeight: stack.height
      clip: true

      ScrollBar.vertical: ScrollBar {}

      StackLayout {
        id: stack

        width: flick.width - flick.leftMargin - flick.rightMargin
        currentIndex: settings.current

        // 0: System
        Column {
          spacing: 24

          SystemCloudLocal {}
          SystemAiModels {}
          SystemStoreTranscriptions {
            divided: false
          }
        }

        // 1: Sound
        Column {
          spacing: 24

          SoundInputSource {
            id: soundInputSource
          }
          SoundEnableSoundEffects {}
          SoundEnableLlm {}
          SoundFxs {
            divided: false
          }
        }

        // 2: Appearance
        Column {
          spacing: 24

          AppearanceTheme {}
          AppearanceOverlaySize {}
          AppearancePosition {
            divided: false
          }
        }

        // 3: Format
        Column {
          spacing: 24

          FormatDictionary {
            id: formatDictionary
          }
          FormatSnippets {
            id: formatSnippets
          }
          FormatModes {
            id: formatModes
          }
        }

        // 4: Keyboard
        Column {
          spacing: 24

          KeyboardPushToTalkToggle {}
          KeyboardShortcuts {
            divided: false
          }
        }

        // 5: Meetings
        Column {
          spacing: 24

          MeetingsMeetings {
            divided: false
          }
        }

        // 6: History
        Column {
          spacing: 24

          HistoryActivity {}
          HistoryStatus {}
          HistoryErrors {}
          HistoryLogs {
            divided: false
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
