// Settings Data - Plain data struct for all user-configurable preferences.

#ifndef SETTINGS_DATA_H
#define SETTINGS_DATA_H

#include <QMap>
#include <QString>

struct SettingsData {
  QString input_source;                 // Audio capture device name or "default"
  QMap<QString, QString> dictionary;   // Original word -> replacement word
  bool markdown_mode = false;
  bool allow_accents = true;
  bool allow_punctuation = true;
};

#endif // SETTINGS_DATA_H
