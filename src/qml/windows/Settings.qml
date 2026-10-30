// Settings - Main menus in the sidebar; each menu's sub-sections on the right. Lazy-loaded.

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Window
import QtQuick.Controls.Basic
import QtQuick.Layouts
import "../components"
import "../components/settings"
import "../components/ui" as Ui

Window {
  id: settings

  readonly property var menus: [qsTr("System"), qsTr("Sound"), qsTr("Appearance"), qsTr("Format"), qsTr("Keyboard"), qsTr("Meetings"), qsTr("History")]
  property int current: 1  // index in menus, Sound by default

  width: 880
  height: 800
  minimumWidth: 640
  minimumHeight: 480
  title: qsTr("Gladys Settings")
  color: "transparent"
  flags: Qt.Window | Qt.FramelessWindowHint
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

  // Rounded pseudoframe: square when maximized, since the window fills the screen.
  Rectangle {
    anchors.fill: parent
    radius: settings.visibility === Window.Maximized ? 0 : Theme.radius
    color: Theme.background
    clip: true
  }

  // Sidebar: the main menus only.
  Column {
    id: sidebar

    anchors.left: parent.left
    anchors.top: titleBar.bottom
    anchors.leftMargin: 12
    anchors.topMargin: 16
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
    anchors.top: titleBar.bottom
    anchors.bottom: parent.bottom

    Flickable {
      id: flick

      anchors.left: parent.left
      anchors.right: parent.right
      anchors.top: parent.top
      anchors.bottom: applyButton.top
      anchors.bottomMargin: 16
      contentWidth: width
      // Current page's implicit height plus 32 px padding top and bottom. StackLayout is as tall as its tallest page.
      contentHeight: (stack.children.length ? stack.children[stack.currentIndex].implicitHeight : 0) + 64
      interactive: contentHeight > height
      clip: true

      ScrollBar.vertical: ScrollBar {
        policy: ScrollBar.AsNeeded
      }

      StackLayout {
        id: stack

        x: 32
        y: 32
        width: flick.width - 64
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

    Ui.Button {
      id: applyButton

      variant: "primary"
      text: qsTr("Apply")
      anchors.left: parent.left
      anchors.right: parent.right
      anchors.bottom: parent.bottom
      anchors.margins: 32
      onClicked: settings.apply()
    }
  }

  Ui.TitleBar {
    id: titleBar

    window: settings
    title: settings.title
    anchors.left: parent.left
    anchors.right: parent.right
    anchors.top: parent.top
  }

  // Resize grips: thin edges and corners. A drag starts a native resize. Off while maximized.
  component Grip: MouseArea {
    required property int edges

    enabled: settings.visibility !== Window.Maximized
    hoverEnabled: true
    onPressed: settings.startSystemResize(edges)
  }

  Grip {
    edges: Qt.LeftEdge
    cursorShape: Qt.SizeHorCursor
    width: 6
    anchors.left: parent.left
    anchors.top: parent.top
    anchors.bottom: parent.bottom
    anchors.topMargin: 12
    anchors.bottomMargin: 12
  }
  Grip {
    edges: Qt.RightEdge
    cursorShape: Qt.SizeHorCursor
    width: 6
    anchors.right: parent.right
    anchors.top: parent.top
    anchors.bottom: parent.bottom
    anchors.topMargin: 12
    anchors.bottomMargin: 12
  }
  Grip {
    edges: Qt.TopEdge
    cursorShape: Qt.SizeVerCursor
    height: 6
    anchors.top: parent.top
    anchors.left: parent.left
    anchors.right: parent.right
    anchors.leftMargin: 12
    anchors.rightMargin: 12
  }
  Grip {
    edges: Qt.BottomEdge
    cursorShape: Qt.SizeVerCursor
    height: 6
    anchors.bottom: parent.bottom
    anchors.left: parent.left
    anchors.right: parent.right
    anchors.leftMargin: 12
    anchors.rightMargin: 12
  }
  Grip {
    edges: Qt.LeftEdge | Qt.TopEdge
    cursorShape: Qt.SizeFDiagCursor
    width: 12
    height: 12
    anchors.left: parent.left
    anchors.top: parent.top
  }
  Grip {
    edges: Qt.RightEdge | Qt.TopEdge
    cursorShape: Qt.SizeBDiagCursor
    width: 12
    height: 12
    anchors.right: parent.right
    anchors.top: parent.top
  }
  Grip {
    edges: Qt.LeftEdge | Qt.BottomEdge
    cursorShape: Qt.SizeBDiagCursor
    width: 12
    height: 12
    anchors.left: parent.left
    anchors.bottom: parent.bottom
  }
  Grip {
    edges: Qt.RightEdge | Qt.BottomEdge
    cursorShape: Qt.SizeFDiagCursor
    width: 12
    height: 12
    anchors.right: parent.right
    anchors.bottom: parent.bottom
  }
}
