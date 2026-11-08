// SystemModelList - LLM / ASR tabs over their models. Select the one Gladys runs; install or uninstall the rest.

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls.Basic
import "../.."
import "../ui" as Ui

Column {
  id: list

  property var rows: []
  readonly property int kind: tabs.currentIndex  // 0 = LLM, 1 = ASR, as ModelKind
  readonly property int installedCount: rows.filter(row => row.installed).length

  width: parent.width
  spacing: Theme.gap

  function reload() {
    rows = modelsBridge.models(tabs.currentIndex);
  }

  Component.onCompleted: reload()

  Connections {
    target: modelsBridge

    function onChanged() {
      error.text = "";
      list.reload();
    }
    function onFailed(file, reason) {
      error.text = reason;
    }
  }

  Ui.Tabs {
    id: tabs

    model: [qsTr("LLM"), qsTr("ASR")]
    onCurrentIndexChanged: {
      error.text = "";
      list.reload();
    }
  }

  Repeater {
    model: list.rows

    delegate: Ui.Box {
      id: row

      required property var modelData
      property int progress: modelData.progress

      width: list.width
      padding: Theme.spacingSm
      color: modelData.selected ? Theme.hover : Theme.surface
      border.color: modelData.selected ? Theme.accent : Theme.border
      radius: Theme.radiusLg

      Behavior on color { ColorAnimation { duration: Theme.durationBase } }
      Behavior on border.color { ColorAnimation { duration: Theme.durationBase } }

      Connections {
        target: modelsBridge

        function onProgressChanged(file, percent) {
          if (file === row.modelData.file) {
            row.progress = percent;
          }
        }
      }

      Item {
        width: parent.width
        height: 32  // the icon button height

        Ui.Text {
          anchors.left: parent.left
          anchors.right: status.left
          anchors.rightMargin: Theme.spacingMd
          anchors.verticalCenter: parent.verticalCenter
          text: row.modelData.name
        }
        Ui.Text {
          id: status

          anchors.right: actions.left
          anchors.rightMargin: Theme.spacingMd
          anchors.verticalCenter: parent.verticalCenter
          muted: true
          text: !row.modelData.selected ? "" : modelsBridge.loading ? qsTr("Loading…") : qsTr("In use")
        }
        Row {
          id: actions

          anchors.right: parent.right
          anchors.verticalCenter: parent.verticalCenter
          spacing: Theme.spacingSm

          Ui.Text {
            visible: row.modelData.busy
            text: qsTr("%1%").arg(row.progress)
          }
          Ui.Button {
            id: installButton

            visible: !row.modelData.busy && !row.modelData.installed
            variant: "icon"
            implicitWidth: 32
            implicitHeight: 32
            iconSource: "qrc:/icons/download.svg"
            ToolTip.visible: hovered
            ToolTip.text: qsTr("Install")
            onClicked: modelsBridge.install(list.kind, row.modelData.file)
          }
          Ui.Button {
            id: selectButton

            visible: !row.modelData.busy && row.modelData.installed && !row.modelData.selected
            variant: "icon"
            implicitWidth: 32
            implicitHeight: 32
            iconSource: "qrc:/icons/check.svg"
            enabled: !modelsBridge.loading
            opacity: enabled ? 1 : 0.4
            ToolTip.visible: hovered
            ToolTip.text: qsTr("Select")
            onClicked: modelsBridge.select(list.kind, row.modelData.file)
          }
          Ui.Button {
            id: uninstallButton

            visible: !row.modelData.busy && row.modelData.installed && !row.modelData.selected
            variant: "icon"
            implicitWidth: 32
            implicitHeight: 32
            iconSource: "qrc:/icons/trash.svg"
            enabled: list.installedCount > 1
            opacity: enabled ? 1 : 0.4
            ToolTip.visible: hovered
            ToolTip.text: qsTr("Uninstall")
            onClicked: modelsBridge.uninstall(list.kind, row.modelData.file)
          }
        }
      }
    }
  }

  Ui.Alert {
    width: list.width
    visible: list.installedCount <= 1
    tone: "warning"
    text: qsTr("Only one model is installed. It cannot be uninstalled, keep at least one.")
  }
  Ui.Alert {
    id: error

    width: list.width
    visible: text !== ""
    tone: "error"
  }
}
