// Setting Module - Persistent user preferences (~/.config/gladys/gladys.ini).
// Composes the menus (Sound, Format). Runs on its own thread: callers reach it with blocking queued calls.

#ifndef SETTING_HPP
#define SETTING_HPP

#include <QObject>
#include <QStringList>
#include <QVariantMap>

#include "format.hpp"
#include "sound.hpp"

class Setting : public QObject {
  Q_OBJECT

 public:
  explicit Setting(QObject* parent = nullptr);

 public slots:
  void publish();  // emits SETTINGS_CHANGED with the current values
  QVariantMap snapshot() const;
  QStringList input_sources() const;
  void apply(const QVariantMap& values);  // validates, persists, then publishes

 signals:
  void SETTINGS_CHANGED(const QVariantMap& snapshot);

 private:
  void load();

  Sound m_sound;
  Format m_format;
};

#endif  // SETTING_HPP
