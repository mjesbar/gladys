// Tray - System tray icon with Settings and Quit menu (Qt Labs Platform).

import QtQuick
import Qt.labs.platform

SystemTrayIcon {
    id: tray

    signal settingsRequested()

    visible: true
    tooltip: "Gladys"
    icon.source: "qrc:/icons/icon-app.svg"

    onActivated: (reason) => {
        if (reason === SystemTrayIcon.Trigger) {
            tray.settingsRequested();
        }
    }

    menu: Menu {
        MenuItem {
            text: qsTr("Settings")
            onTriggered: tray.settingsRequested()
        }
        MenuItem {
            text: qsTr("Quit")
            onTriggered: Qt.quit()
        }
    }
}
