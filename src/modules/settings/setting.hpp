// Setting Module - User preferences. In-memory stores, persisted through Data (history.db).
// Runs on its own thread: callers reach it with blocking queued calls.

#ifndef SETTING_HPP
#define SETTING_HPP

#include <QObject>
#include <QStringList>
#include <QVariantMap>

#include "appearance.hpp"
#include "format.hpp"
#include "history.hpp"
#include "keyboard.hpp"
#include "sound.hpp"

class Setting : public QObject {
  Q_OBJECT

 public:
  explicit Setting(QObject* parent = nullptr);

 public slots:
  void load();     // fills the in-memory stores from Data; call once, before publish
  void publish();  // emits SETTINGS_CHANGED with the current values
  QVariantMap snapshot() const;
  QStringList input_sources() const;
  void apply(const QVariantMap& values);  // validates, updates memory, persists the diff, then publishes

 signals:
  void SETTINGS_CHANGED(const QVariantMap& snapshot);

 private:
  Sound m_sound;
  Format m_format;
  History m_history;
  Appearance m_appearance;
  Keyboard m_keyboard;
};

#endif  // SETTING_HPP
