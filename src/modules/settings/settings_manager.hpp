// Settings Manager - Singleton managing persistent user preferences.
// Reads/writes QSettings to ~/.config/gladys/gladys.ini.
// Provides platform-specific audio input source enumeration.

#ifndef SETTINGS_MANAGER_H
#define SETTINGS_MANAGER_H

#include <QObject>
#include <QSettings>
#include <QString>
#include <QStringList>

#include "settings_data.hpp"

class SettingsManager : public QObject {
  Q_OBJECT

public:
  static SettingsManager *instance();

  void load();
  void save();
  void reset_to_defaults();

  // Accessors
  QString input_source() const;
  QMap<QString, QString> dictionary() const;
  bool markdown_mode() const;
  bool allow_accents() const;
  bool allow_punctuation() const;
  SettingsData data() const;

  // Mutators (call save() internally)
  void set_input_source(const QString &s);
  void set_dictionary(const QMap<QString, QString> &d);
  void set_markdown_mode(bool v);
  void set_allow_accents(bool v);
  void set_allow_punctuation(bool v);

  // Platform-specific audio input source listing
  static QStringList available_input_sources();
  static QString default_input_source();

signals:
  void settings_changed();

private:
  explicit SettingsManager(QObject *parent = nullptr);
  ~SettingsManager() override;

  static SettingsManager *s_instance;
  SettingsData m_data;

  void apply_from_settings(QSettings &s);
  void write_to_settings(QSettings &s) const;
};

#endif // SETTINGS_MANAGER_H
