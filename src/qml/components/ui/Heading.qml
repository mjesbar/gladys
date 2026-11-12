// Heading - Bold title, level 1 (largest) to 4 (smallest).

import "../.."

Text {
  property int level: 2
  readonly property var sizes: [Theme.fontXl, Theme.fontLg, Theme.fontMd, Theme.fontSm]

  size: sizes[level - 1]
  font.family: Theme.fontDisplay
  font.bold: true
}
