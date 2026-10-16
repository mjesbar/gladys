// Setting Module - Persistent user preferences (~/.config/gladys/gladys.ini).
// Setters only change memory; save() persists and emits SETTINGS_CHANGED.

#ifndef SETTING_HPP
#define SETTING_HPP

#include <QMap>
#include <QObject>
#include <QString>
#include <QStringList>

class Setting : public QObject {
  Q_OBJECT

public:
  static Setting *instance();

  void save();

  QString input_source() const;
  QMap<QString, QString> dictionary() const;
  bool markdown_mode() const;
  bool allow_accents() const;
  bool allow_punctuation() const;

  void set_input_source(const QString &v);
  void set_dictionary(const QMap<QString, QString> &d);
  void set_markdown_mode(bool v);
  void set_allow_accents(bool v);
  void set_allow_punctuation(bool v);

  static QStringList available_input_sources();

signals:
  void SETTINGS_CHANGED(const QString &input_source);

private:
  explicit Setting(QObject *parent = nullptr);
  void load();

  static Setting *s_instance;
  QString m_input_source = "default"; // capture device name or "default"
  QMap<QString, QString> m_dictionary; // original word -> replacement
  bool m_markdown_mode = false;
  bool m_allow_accents = true;
  bool m_allow_punctuation = true;
};

#endif // SETTING_HPP
