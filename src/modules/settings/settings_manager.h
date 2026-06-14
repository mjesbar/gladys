// Settings Manager - Singleton managing persistent user preferences.
// Reads/writes QSettings to ~/.config/gladys/gladys.ini.
// Provides platform-specific audio input source enumeration.

#ifndef SETTINGS_MANAGER_H
#define SETTINGS_MANAGER_H

#include <QObject>
#include <QSettings>
#include <QString>
#include <QStringList>

#include "settings_data.h"

class SettingsManager : public QObject {
  Q_OBJECT

public:
  static SettingsManager *instance();

  void load();
  void save();
  void resetToDefaults();

  // Accessors
  QString inputSource() const { return m_data.inputSource; }
  QMap<QString, QString> dictionary() const { return m_data.dictionary; }
  bool markdownMode() const { return m_data.markdownMode; }
  bool allowAccents() const { return m_data.allowAccents; }
  bool allowPunctuation() const { return m_data.allowPunctuation; }
  SettingsData data() const { return m_data; }

  // Mutators (call save() internally)
  void setInputSource(const QString &s);
  void setDictionary(const QMap<QString, QString> &d);
  void setMarkdownMode(bool v);
  void setAllowAccents(bool v);
  void setAllowPunctuation(bool v);

  // Platform-specific audio input source listing
  static QStringList availableInputSources();
  static QString defaultInputSource();

signals:
  void settingsChanged();

private:
  explicit SettingsManager(QObject *parent = nullptr);
  ~SettingsManager() override;

  static SettingsManager *s_instance;
  SettingsData m_data;

  void applyFromSettings(QSettings &s);
  void writeToSettings(QSettings &s) const;
};

#endif // SETTINGS_MANAGER_H
