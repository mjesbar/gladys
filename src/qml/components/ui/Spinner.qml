// Spinner - Rotating loading icon.

import QtQuick

Icon {
  source: "qrc:/icons/spinner.svg"

  NumberAnimation on rotation {
    from: 0
    to: 360
    duration: 900
    loops: Animation.Infinite
  }
}
