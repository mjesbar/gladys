// Settings Bridge - Exposes the Setting module to the Settings QML window.

#ifndef UI_SETTINGS_BRIDGE_HPP
#define UI_SETTINGS_BRIDGE_HPP

#include <QObject>
#include <QStringList>
#include <QVariantMap>

class Setting;

class SettingsBridge : public QObject {
  Q_OBJECT
  Q_PROPERTY(QStringList inputSources READ input_sources CONSTANT)
  Q_PROPERTY(QStringList styles READ styles CONSTANT)

 public:
  // setting lives on the settings thread; every call blocks until it answers.
  explicit SettingsBridge(Setting* setting, QObject* parent = nullptr);

  QStringList input_sources() const;
  QStringList styles() const;

  Q_INVOKABLE QVariantMap load() const;
  Q_INVOKABLE void apply(const QVariantMap& values);

 private:
  Setting* m_setting;
};

#endif  // UI_SETTINGS_BRIDGE_HPP
