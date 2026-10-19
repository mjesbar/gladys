// Settings Bridge - Exposes the Setting module to the Settings QML window.

#ifndef UI_SETTINGS_BRIDGE_HPP
#define UI_SETTINGS_BRIDGE_HPP

#include <QObject>
#include <QStringList>
#include <QVariantMap>

class SettingsBridge : public QObject {
  Q_OBJECT
  Q_PROPERTY(QStringList inputSources READ input_sources CONSTANT)

 public:
  explicit SettingsBridge(QObject* parent = nullptr);

  QStringList input_sources() const;

  Q_INVOKABLE QVariantMap load() const;
  Q_INVOKABLE void apply(const QVariantMap& values);
};

#endif  // UI_SETTINGS_BRIDGE_HPP
