// Settings - Input source, dictionary, snippets and style flags. Lazy-loaded.

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

    Flickable {
        anchors.fill: parent
        contentHeight: content.height + 64
        clip: true

        ScrollBar.vertical: ScrollBar {}

        Column {
            id: content

            x: 32
            y: 32
            width: parent.width - 64
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

            AliasCard {
                width: parent.width
                title: qsTr("Dictionary")
                toHint: qsTr("Brief description")
                aliasModel: dictionaryModel
            }

            AliasCard {
                width: parent.width
                title: qsTr("Snippets")
                aliasModel: snippetModel
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
