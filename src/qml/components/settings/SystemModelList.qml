// SystemModelList - LLM / ASR tabs over their models. Install or uninstall per row; one model always stays.

pragma ComponentBehavior: Bound

import QtQuick
import ".."
import "../.."
import "../ui" as Ui

Column {
    id: list

    property var rows: []
    readonly property int kind: tabs.currentIndex  // 0 = LLM, 1 = ASR, as ModelKind
    readonly property int installedCount: rows.filter(row => row.installed).length

    width: parent.width
    spacing: Theme.spacingMd

    function reload() {
        rows = modelsBridge.models(kind);
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

        delegate: Item {
            id: row

            required property var modelData

            property int progress: modelData.progress

            width: list.width
            height: action.height

            Connections {
                target: modelsBridge

                function onProgressChanged(file, percent) {
                    if (file === row.modelData.file) {
                        row.progress = percent;
                    }
                }
            }

            Ui.Text {
                anchors.left: parent.left
                anchors.right: status.left
                anchors.rightMargin: Theme.spacingMd
                anchors.verticalCenter: parent.verticalCenter
                text: row.modelData.name
            }
            Ui.Text {
                id: status

                anchors.right: action.left
                anchors.rightMargin: Theme.spacingMd
                anchors.verticalCenter: parent.verticalCenter
                muted: true
                text: row.modelData.inUse ? qsTr("In use") : ""
            }
            Ui.Button {
                id: action

                anchors.right: parent.right
                anchors.verticalCenter: parent.verticalCenter
                variant: row.modelData.installed ? "ghost" : "primary"
                text: row.modelData.busy ? qsTr("Installing %1%").arg(row.progress) : row.modelData.installed ? qsTr("Uninstall") : qsTr("Install")
                enabled: !row.modelData.busy && (!row.modelData.installed || (!row.modelData.inUse && list.installedCount > 1))
                onClicked: row.modelData.installed ? modelsBridge.uninstall(list.kind, row.modelData.file) : modelsBridge.install(list.kind, row.modelData.file)
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
