// Settings Data - Plain data struct for all user-configurable preferences.

#ifndef SETTINGS_DATA_H
#define SETTINGS_DATA_H

#include <QMap>
#include <QString>

struct SettingsData {
  QString inputSource;                 // Audio capture device name or "default"
  QMap<QString, QString> dictionary;   // Original word -> replacement word
  bool markdownMode = false;
  bool allowAccents = true;
  bool allowPunctuation = true;
};

#endif // SETTINGS_DATA_H
