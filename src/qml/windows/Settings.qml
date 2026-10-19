// Settings - Input source, dictionary aliases and style flags. Lazy-loaded.

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Window
import QtQuick.Controls.Basic
import "../components"

Window {
    id: settings

    width: 800
    height: 800
    minimumWidth: 480
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
        id: aliasModel
    }

    function load() {
        const s = settingsBridge.load();
        const index = inputCombo.find(s.inputSource);
        inputCombo.currentIndex = index >= 0 ? index : 0;
        aliasModel.clear();
        for (const from of Object.keys(s.dictionary)) {
            aliasModel.append({
                from: from,
                to: s.dictionary[from]
            });
        }
        markdownCombo.currentIndex = s.markdown ? 0 : 1;
        styleCombo.currentIndex = Math.max(0, styleCombo.find(s.style));
    }

    function addAlias() {
        const from = fromField.text.trim();
        if (from === "") {
            return;
        }
        aliasModel.append({
            from: from,
            to: toField.text.trim()
        });
        fromField.clear();
        toField.clear();
        fromField.forceActiveFocus();
    }

    function apply() {
        const dictionary = {};
        for (let i = 0; i < aliasModel.count; ++i) {
            const row = aliasModel.get(i);
            dictionary[row.from] = row.to;
        }
        settingsBridge.apply({
            inputSource: inputCombo.currentText,
            style: styleCombo.currentText,
            markdown: markdownCombo.currentIndex === 0,
            dictionary: dictionary
        });
    }

    Item {
        anchors.fill: parent

        Column {
            anchors.fill: parent
            anchors.margins: 32
            spacing: 16

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

            Card {
                width: parent.width
                title: qsTr("Dictionary")

                ListView {
                    id: aliasView

                    width: parent.width
                    height: 180
                    clip: true
                    model: aliasModel

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
                            onEditingFinished: aliasModel.setProperty(row.index, "from", text.trim())
                        }
                        Field {
                            width: (row.width - 52) / 2
                            height: parent.height
                            text: row.to
                            onEditingFinished: aliasModel.setProperty(row.index, "to", text.trim())
                        }
                        IconButton {
                            y: (parent.height - height) / 2
                            iconSource: "qrc:/icons/trash.svg"
                            ToolTip.visible: hovered
                            ToolTip.text: qsTr("Remove")
                            onClicked: aliasModel.remove(row.index)
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
                        onAccepted: settings.addAlias()
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
                        placeholderText: qsTr("Replacement")
                        onAccepted: settings.addAlias()
                    }
                    IconButton {
                        iconSource: "qrc:/icons/plus.svg"
                        ToolTip.visible: hovered
                        ToolTip.text: qsTr("Add Alias")
                        onClicked: settings.addAlias()
                    }
                }
            }

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

            Button {
                id: applyButton

                width: parent.width
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
}
