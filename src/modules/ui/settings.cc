// Settings Bridge - Exposes the Setting module to the Settings QML window.

#include "settings.hpp"

#include <QMetaObject>

#include "../settings/format.hpp"
#include "../settings/setting.hpp"

SettingsBridge::SettingsBridge(Setting* setting, QObject* parent)
    : QObject(parent), m_setting(setting) {}

QStringList SettingsBridge::input_sources() const {
  QStringList sources;
  QMetaObject::invokeMethod(m_setting, "input_sources", Qt::BlockingQueuedConnection,
                            Q_RETURN_ARG(QStringList, sources));
  return sources;
}

QStringList SettingsBridge::styles() const { return Format::available_styles(); }

QVariantMap SettingsBridge::load() const {
  QVariantMap values;
  QMetaObject::invokeMethod(m_setting, "snapshot", Qt::BlockingQueuedConnection,
                            Q_RETURN_ARG(QVariantMap, values));
  return values;
}

void SettingsBridge::apply(const QVariantMap& values) {
  QMetaObject::invokeMethod(m_setting, "apply", Qt::BlockingQueuedConnection,
                            Q_ARG(QVariantMap, values));
}
