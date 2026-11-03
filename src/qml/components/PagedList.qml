// PagedList - Newest-first rows, 20 per page. The next page loads when the list reaches its end.

pragma ComponentBehavior: Bound

import QtQuick

ListView {
  id: list

  property var fetch  // function(offset, limit) -> array of rows
  property int pageSize: 20
  property bool exhausted: false

  width: parent.width
  height: 240
  clip: true
  model: ListModel {
    id: rows
  }

  onAtYEndChanged: if (atYEnd) more()

  function reset() {
    rows.clear();
    exhausted = false;
    more();
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
