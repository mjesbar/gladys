// Settings - Main menus in the sidebar; each menu's sub-sections on the right. Lazy-loaded.

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Window
import QtQuick.Controls.Basic
import QtQuick.Layouts
import ".."
import "../components"
import "../components/settings"
import "../components/ui" as Ui

Window {
  id: settings

  readonly property var menus: [qsTr("System"), qsTr("Sound"), qsTr("Appearance"), qsTr("Format"), qsTr("Keyboard"), qsTr("Meetings"), qsTr("History")]
  property int current: 1  // index in menus, Sound by default

  readonly property int sidebarWidth: 220

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

  // Live updates while the window is open; a hidden window reads nothing.
  Connections {
    target: historyBridge

    function onTranscriptionsChanged() {
      if (settings.visible) {
        historyActivity.refresh();
      }
    }
    function onLogsChanged() {
      if (settings.visible) {
        historyLogs.refresh();
        historyStatus.reload();
        historyErrors.reload();
      }
    }
  }

  function load() {
    const s = settingsBridge.load();
    soundInputSource.load(s);
    formatDictionary.load(s);
    formatSnippets.load(s);
    formatModes.load(s);
    historyActivity.load(s);
    historyLogs.load(s);
    historyActivity.reload();
    historyStatus.reload();
    historyErrors.reload();
    historyLogs.reload();
  }

  Item {
    id: frame

    anchors.fill: parent

    Rectangle {
      anchors.fill: parent
      radius: settings.visibility === Window.Maximized ? 0 : Theme.radiusLg
      color: Theme.background
    }

    // Sidebar: full height behind the transparent title bar. Its rounded edge is
    // the window corner radius; the square edge strip hides its rounded right side.

    Rectangle {
      id: sidebarPanel

      anchors.left: parent.left
      anchors.top: parent.top
      anchors.bottom: parent.bottom
      width: settings.sidebarWidth + Theme.radiusLg
      radius: settings.visibility === Window.Maximized ? 0 : Theme.radiusLg
      color: Theme.sidebar
    }
    Rectangle {
      x: settings.sidebarWidth
      anchors.top: parent.top
      anchors.bottom: parent.bottom
      width: Theme.radiusLg
      color: Theme.background
    }

    // Sidebar: the main menus only.
    Column {
      id: sidebar

      anchors.left: parent.left
      anchors.top: titleBar.bottom
      anchors.leftMargin: Theme.spacingMd
      anchors.topMargin: Theme.spacingLg
      width: 196
      spacing: Theme.spacingXs

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

          HoverHandler {
            cursorShape: Qt.PointingHandCursor
          }

          background: Rectangle {
            radius: Theme.radiusMd
            color: Theme.hover  // fixed fill: only opacity animates, so no colour flicker
            opacity: menuItem.highlighted ? 1 : menuItem.hovered ? 0.5 : 0

            Behavior on opacity { NumberAnimation { duration: Theme.durationBase } }
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

    // Page area: config panel, darker grey than the sidebar. Each page gets its padding from the StackLayout.
    Item {
      anchors.left: parent.left
      anchors.leftMargin: settings.sidebarWidth
      anchors.right: parent.right
      anchors.top: titleBar.bottom
      anchors.bottom: parent.bottom

      Flickable {
        id: flick

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        contentWidth: width
        // Current page's implicit height plus padding top and bottom. StackLayout is as tall as its tallest page.
        contentHeight: (stack.children.length ? stack.children[stack.currentIndex].implicitHeight : 0) + 2 * Theme.spacingXxl
        // While the pointer is over a list, the list takes the wheel and the page never scrolls, even at its ends.
      interactive: contentHeight > height && !(formatDictionary.hovered || formatSnippets.hovered || historyActivity.hovered || historyLogs.hovered)
        clip: true

        ScrollBar.vertical: ScrollBar {
          policy: ScrollBar.AsNeeded
        }

        StackLayout {
          id: stack

          x: Theme.spacingXxl
          y: Theme.spacingXxl
          width: flick.width - 2 * Theme.spacingXxl
          currentIndex: settings.current

          // 0: System
          Column {
            spacing: Theme.spacingXl

            SystemCloudLocal {}
            SystemAiModels {
              divided: false
            }
          }

          // 1: Sound
          Column {
            spacing: Theme.spacingXl

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
            spacing: Theme.spacingXl

            AppearanceTheme {}
            AppearanceOverlaySize {}
            AppearancePosition {
              divided: false
            }
          }

          // 3: Format
          Column {
            spacing: Theme.spacingXl

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
            spacing: Theme.spacingXl

            KeyboardPushToTalkToggle {}
            KeyboardShortcuts {
              divided: false
            }
          }

          // 5: Meetings
          Column {
            spacing: Theme.spacingXl

            MeetingsMeetings {
              divided: false
            }
          }

          // 6: History
          Column {
            spacing: Theme.spacingXl

            HistoryActivity {
              id: historyActivity
            }
            HistoryStatus {
              id: historyStatus
            }
            HistoryErrors {
              id: historyErrors
            }
            HistoryLogs {
              id: historyLogs

              divided: false
            }
          }
        }
      }
    }

    Ui.TitleBar {
      id: titleBar

      z: 1  // paints above sidebar and config panel

      window: settings
      title: settings.title
      anchors.left: parent.left
      anchors.right: parent.right
      anchors.top: parent.top
    }

    // Window hairline: 1px stroke over the frame edge, hidden while maximized.
    Rectangle {
      anchors.fill: parent
      radius: Theme.radiusLg
      color: "transparent"
      border.color: Theme.border
      border.width: 1
      visible: settings.visibility !== Window.Maximized
      z: 2
    }

    // Resize grips: thin edges and corners. A drag starts a native resize. Off while maximized.
    component Grip: MouseArea {
      required property int edges

      z: 3  // above titleBar (z 1), so top edge and corners receive the press
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
}
