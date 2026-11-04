// PagedList - Newest-first rows, 20 per page. The next page loads when the list reaches its end.

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls.Basic
import ".."

ListView {
  id: list

  property var fetch  // function(offset, limit) -> array of rows
  property int pageSize: 20
  property bool exhausted: false
  readonly property bool hovered: hover.hovered

  HoverHandler {
    id: hover
  }

  width: parent.width
  height: 240
  spacing: Theme.spacingSm
  clip: true
  model: ListModel {
    id: rows
  }

  ScrollBar.vertical: ScrollBar {
    policy: ScrollBar.AsNeeded
    width: Theme.spacingMd
  }

  onAtYEndChanged: if (atYEnd) more()

  function reset() {
    rows.clear();
    exhausted = false;
    more();
  }

  // Prepends rows stored since the newest one shown. Ids only grow, so id > top means new.
  function refresh() {
    const top = rows.count > 0 ? rows.get(0).id : 0;
    const fresh = fetch(0, pageSize).filter(row => row.id > top);
    for (let i = fresh.length - 1; i >= 0; --i) {
      rows.insert(0, fresh[i]);
    }
  }

  function more() {
    if (exhausted || !fetch) {
      return;
    }
    const page = fetch(rows.count, pageSize);
    for (const row of page) {
      rows.append(row);
    }
    exhausted = page.length < pageSize;
  }
}
