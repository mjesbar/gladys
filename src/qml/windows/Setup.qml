// Setup - First-run dialog. Downloads the missing default LLM and ASR models, then hands over to the Splash.

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Window
import ".."
import "../components/ui" as Ui

Window {
    id: setup

    signal finished()  // models ready and Open Gladys was chosen, by the user or the countdown
    signal aborted()   // quit or closed before the models were ready

    property int kind: 0  // ModelKind being fetched: 0 LLM, 1 ASR
    property string file: ""  // catalog key being fetched
    readonly property string label: kind === 0 ? "LLM" : "ASR"
    property int percent: 0
    property string reason: ""  // set when the download fails
    property bool done: false  // every default model is installed
    property int seconds: 10  // countdown before Open Gladys is chosen for the user
    property bool confirmed: false
    readonly property bool extracting: percent === 100 && !done && reason === ""

    width: 360
    height: titleBar.height + content.height + 2 * Theme.padding
    title: qsTr("Gladys Setup")
    color: "transparent"
    flags: Qt.Window | Qt.FramelessWindowHint
    transientParent: null
    visible: true

    onClosing: setup.aborted()

    Component.onCompleted: setup.fetch()

    Connections {
        target: modelsBridge

        function onProgressChanged(key, value) {
            if (key === setup.file) {
                setup.percent = value;
            }
        }
        function onFailed(key, cause) {
            if (key === setup.file) {
                setup.reason = cause;
            }
        }
        function onChanged() {
            if (!setup.done && setup.defaultRow(setup.kind).installed) {
                setup.fetch();
            }
        }
    }

    Timer {
        interval: 1000
        repeat: true
        running: setup.done && setup.seconds > 0
        onTriggered: {
            setup.seconds -= 1;
            if (setup.seconds === 0) {
                setup.confirm();
            }
        }
    }

    function defaultRow(k) {
        return modelsBridge.models(k).find(row => row.default);
    }

    // Installs the first default model still missing; when none is, all are installed.
    function fetch() {
        for (const k of [0, 1]) {
            const row = setup.defaultRow(k);
            if (!row.installed) {
                setup.kind = k;
                setup.file = row.file;
                setup.percent = 0;
                setup.reason = "";
                modelsBridge.install(k, row.file);
                return;
            }
        }
        setup.done = true;
    }

    function confirm() {
        setup.seconds = 0;
        setup.confirmed = true;
        setup.finished();
    }

    Rectangle {
        anchors.fill: parent
        radius: Theme.radiusLg
        color: Theme.background
        border.color: Theme.border
        border.width: 1
    }

    Ui.TitleBar {
        id: titleBar

        window: setup
        title: setup.title
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
    }

    // Every row keeps its slot: state changes swap contents, never the layout.
    Column {
        id: content

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: titleBar.bottom
        anchors.margins: Theme.padding
        spacing: Theme.gap

        Ui.Heading {
            level: 3
            width: parent.width
            height: 24
            horizontalAlignment: Text.AlignHCenter
            text: setup.reason !== "" ? qsTr("Download failed") : setup.done ? qsTr("Setup complete") : setup.extracting ? qsTr("Extracting %1 model").arg(setup.label) : qsTr("Downloading %1 model").arg(setup.label)
        }

        Ui.Text {
            width: parent.width
            height: 18
            muted: true
            horizontalAlignment: Text.AlignHCenter
            text: setup.reason === "" && !setup.done ? qsTr("Step %1 of 2").arg(setup.kind + 1) : ""
        }

        Rectangle {
            width: parent.width
            height: 6
            radius: 3
            color: Theme.border

            Rectangle {
                width: parent.width * setup.percent / 100
                height: parent.height
                radius: parent.radius
                color: Theme.accent

                Behavior on width {
                    NumberAnimation {
                        duration: Theme.durationBase
                    }
                }
            }
        }

        Ui.Text {
            width: parent.width
            height: 18
            horizontalAlignment: Text.AlignHCenter
            text: setup.reason === "" && !setup.done ? qsTr("%1%").arg(setup.percent) : ""
        }

        // Centred slot: the extraction spinner, Open Gladys with its countdown, or the error.
        Item {
            width: parent.width
            height: 40

            Ui.Spinner {
                anchors.centerIn: parent
                size: 24
                visible: setup.extracting
            }
            Ui.Button {
                anchors.centerIn: parent
                implicitHeight: 24
                leftPadding: Theme.spacingMd
                visible: setup.done
                variant: "primary"
                text: qsTr("Open Gladys (%1s)").arg(setup.seconds)
                onClicked: setup.confirm()
            }
            Row {
                anchors.centerIn: parent
                visible: setup.reason !== ""
                spacing: Theme.gap

                Rectangle {
                    width: 32
                    height: 32
                    radius: 16
                    color: Theme.danger

                    Ui.Text {
                        anchors.centerIn: parent
                        text: "!"
                        size: Theme.fontLg
                        color: Theme.background
                    }
                }
                Ui.Text {
                    width: 276
                    height: 32
                    verticalAlignment: Text.AlignVCenter
                    text: setup.reason
                    color: Theme.danger
                }
            }
        }

        // Actions slot: Quit and Retry on error. Hidden otherwise, so no space is left empty.
        Item {
            visible: setup.reason !== ""
            width: parent.width
            height: 40

            Row {
                anchors.centerIn: parent
                spacing: Theme.spacingSm

                Ui.Button {
                    implicitHeight: 24
                    leftPadding: Theme.spacingMd
                    text: qsTr("Quit")
                    onClicked: setup.close()
                }
                Ui.Button {
                    implicitHeight: 24
                    leftPadding: Theme.spacingMd
                    variant: "primary"
                    text: qsTr("Retry")
                    onClicked: setup.fetch()
                }
            }
        }
    }
}
