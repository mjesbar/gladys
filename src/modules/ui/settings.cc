// Settings Bridge - Exposes the Setting module to the Settings QML window.

#include "settings.hpp"

#include <QMetaObject>

#include "../settings/appearance.hpp"
#include "../settings/format.hpp"
#include "../settings/setting.hpp"
#include "../settings/sound.hpp"

SettingsBridge::SettingsBridge(Setting* setting, QObject* parent)
    : QObject(parent), m_setting(setting) {}

QStringList SettingsBridge::sfxs() const { return Sound::available_sfx(); }

QStringList SettingsBridge::input_sources() const {
  QStringList sources;
  QMetaObject::invokeMethod(
    m_setting, "input_sources", Qt::BlockingQueuedConnection, Q_RETURN_ARG(QStringList, sources));
  return sources;
}

QStringList SettingsBridge::styles() const { return Format::available_styles(); }

QStringList SettingsBridge::positions() const { return Appearance::available_positions(); }

QStringList SettingsBridge::overlay_sizes() const { return Appearance::available_sizes(); }

QStringList SettingsBridge::screens() const { return Appearance::available_screens(); }

QVariantMap SettingsBridge::load() const {
  QVariantMap values;
  QMetaObject::invokeMethod(
    m_setting, "snapshot", Qt::BlockingQueuedConnection, Q_RETURN_ARG(QVariantMap, values));
  return values;
}

void SettingsBridge::apply(const QVariantMap& values) {
  QMetaObject::invokeMethod(
    m_setting, "apply", Qt::BlockingQueuedConnection, Q_ARG(QVariantMap, values));
}
