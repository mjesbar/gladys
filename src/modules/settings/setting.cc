// Setting Module - User preferences. In-memory stores, persisted through Data (history.db).

#include "setting.hpp"

#include <QByteArray>
#include <QMetaObject>

#include "../data/data.hpp"
#include "../log/log.hpp"

// Blocking calls into Data: the settings thread waits for the answer. A missing Data returns false.
static bool call_setting(const char* slot, const QString& key, const QVariant& value) {
  bool ok = false;
  QMetaObject::invokeMethod(Data::instance(),
    slot,
    Qt::BlockingQueuedConnection,
    Q_RETURN_ARG(bool, ok),
    Q_ARG(QString, key),
    Q_ARG(QVariant, value));
  return ok;
}

static bool call_pair(const char* slot, const QString& from_text, const QString& to_text) {
  bool ok = false;
  QMetaObject::invokeMethod(Data::instance(),
    slot,
    Qt::BlockingQueuedConnection,
    Q_RETURN_ARG(bool, ok),
    Q_ARG(QString, from_text),
    Q_ARG(QString, to_text));
  return ok;
}

static bool call_delete(const char* slot, const QString& key) {
  bool ok = false;
  QMetaObject::invokeMethod(Data::instance(),
    slot,
    Qt::BlockingQueuedConnection,
    Q_RETURN_ARG(bool, ok),
    Q_ARG(QString, key));
  return ok;
}

// Dictionary and snippets: new keys are created, changed values updated, removed keys deleted.
static void sync_pairs(const char* kind, const QVariantMap& before, const QVariantMap& after) {
  const QByteArray create = QByteArray("create_") + kind;
  const QByteArray update = QByteArray("update_") + kind;
  const QByteArray remove = QByteArray("delete_") + kind;
  for (auto it = after.cbegin(); it != after.cend(); ++it) {
    if (!before.contains(it.key())) {
      call_pair(create.constData(), it.key(), it.value().toString());
    } else if (before.value(it.key()) != it.value()) {
      call_pair(update.constData(), it.key(), it.value().toString());
    }
  }
  for (auto it = before.cbegin(); it != before.cend(); ++it) {
    if (!after.contains(it.key())) {
      call_delete(remove.constData(), it.key());
    }
  }
}

// Position, overlay size and screen share one row, so they are written together.
static void persist_appearance(const QVariantMap& before, const QVariantMap& after) {
  if (before.value("position") == after.value("position") &&
    before.value("overlaySize") == after.value("overlaySize") &&
    before.value("screen") == after.value("screen")) {
    return;
  }
  QMetaObject::invokeMethod(Data::instance(),
    "update_appearance",
    Qt::BlockingQueuedConnection,
    Q_ARG(QString, after.value("position").toString()),
    Q_ARG(QString, after.value("overlaySize").toString()),
    Q_ARG(QString, after.value("screen").toString()));
}

// Only changed scalars are written: update first, create when the row does not exist yet.
static void persist(const QVariantMap& before, const QVariantMap& after) {
  for (auto it = after.cbegin(); it != after.cend(); ++it) {
    if (it.key() == "dictionary" || it.key() == "snippets" || it.key() == "position" ||
      it.key() == "overlaySize" || it.key() == "screen" || before.value(it.key()) == it.value()) {
      continue;
    }
    if (!call_setting("update_setting", it.key(), it.value())) {
      call_setting("create_setting", it.key(), it.value());
    }
  }
  sync_pairs("dictionary", before.value("dictionary").toMap(), after.value("dictionary").toMap());
  sync_pairs("snippets", before.value("snippets").toMap(), after.value("snippets").toMap());
  persist_appearance(before, after);
}

Setting::Setting(QObject* parent) : QObject(parent) {}

void Setting::load() {
  QVariantMap stored;
  QVariantMap dictionary;
  QVariantMap snippets;
  QVariantMap appearance;
  QMetaObject::invokeMethod(Data::instance(),
    "read_settings",
    Qt::BlockingQueuedConnection,
    Q_RETURN_ARG(QVariantMap, stored));
  QMetaObject::invokeMethod(Data::instance(),
    "read_dictionary",
    Qt::BlockingQueuedConnection,
    Q_RETURN_ARG(QVariantMap, dictionary));
  QMetaObject::invokeMethod(Data::instance(),
    "read_snippets",
    Qt::BlockingQueuedConnection,
    Q_RETURN_ARG(QVariantMap, snippets));
  QMetaObject::invokeMethod(Data::instance(),
    "read_appearance",
    Qt::BlockingQueuedConnection,
    Q_RETURN_ARG(QVariantMap, appearance));
  stored.insert("dictionary", dictionary);
  stored.insert("snippets", snippets);
  stored.insert(appearance);

  m_sound.from_map(stored);
  m_format.from_map(stored);
  m_history.from_map(stored);
  m_appearance.from_map(stored);
  qCInfo(lcSetting) << "loaded from history.db";
}

void Setting::publish() { emit SETTINGS_CHANGED(snapshot()); }

QVariantMap Setting::snapshot() const {
  QVariantMap map = m_sound.to_map();
  map.insert(m_format.to_map());
  map.insert(m_history.to_map());
  map.insert(m_appearance.to_map());
  return map;
}

QStringList Setting::input_sources() const { return Sound::input_sources(); }

void Setting::apply(const QVariantMap& values) {
  const QVariantMap before = snapshot();
  m_sound.from_map(values);
  m_format.from_map(values);
  m_history.from_map(values);
  m_appearance.from_map(values);
  persist(before, snapshot());

  publish();
}
