// Divider - 1px rule, horizontal by default.

import QtQuick
import ".."

Rectangle {
  property bool vertical: false

  color: Theme.border
  implicitWidth: vertical ? 1 : 100
  implicitHeight: vertical ? 100 : 1
}
