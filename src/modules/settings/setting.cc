// Setting Module - Persistent user preferences (~/.config/gladys/gladys.ini).

#include "setting.hpp"

#include <QSettings>

#include "../log/log.hpp"

Setting::Setting(QObject* parent) : QObject(parent) { load(); }

void Setting::load() {
  QSettings s("gladys", "gladys");
  qCInfo(lcSetting) << "loaded" << s.fileName();
  m_sound.load(s);
  m_format.load(s);
}

void Setting::publish() { emit SETTINGS_CHANGED(snapshot()); }

QVariantMap Setting::snapshot() const {
  QVariantMap map = m_sound.to_map();
  map.insert(m_format.to_map());
  return map;
}

QStringList Setting::input_sources() const { return Sound::input_sources(); }

void Setting::apply(const QVariantMap& values) {
  m_sound.from_map(values);
  m_format.from_map(values);

  QSettings s("gladys", "gladys");
  m_sound.save(s);
  m_format.save(s);

  publish();
}
