// Heading - Bold title, level 1 (largest) to 4 (smallest).

import ".."

Text {
  property int level: 2
  readonly property var sizes: [22, 18, 15, 13]

  size: sizes[level - 1]
  font.bold: true
}
