// Flex - Row that wraps, or a column when vertical is true.

import QtQuick
import ".."

Flow {
  property bool vertical: false

  flow: vertical ? Flow.TopToBottom : Flow.LeftToRight
  spacing: Theme.gap
}
