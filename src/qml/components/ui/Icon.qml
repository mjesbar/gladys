// Icon - Square SVG icon at a fixed size.

import QtQuick

Image {
  property int size: 18

  width: size
  height: size
  sourceSize: Qt.size(size * 2, size * 2)
  fillMode: Image.PreserveAspectFit
}
