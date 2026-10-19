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
  static Setting* instance();

  void save();

  QString input_source() const;
  QMap<QString, QString> dictionary() const;
  bool markdown_mode() const;
  QString style() const;

  void set_input_source(const QString& v);
  void set_dictionary(const QMap<QString, QString>& d);
  void set_markdown_mode(bool v);
  void set_style(const QString& v);

  static QStringList available_input_sources();
  static QStringList available_styles();

 signals:
  void SETTINGS_CHANGED(const QString& input_source);

 private:
  explicit Setting(QObject* parent = nullptr);
  void load();

  static Setting* s_instance;
  QString m_input_source = "default";   // capture device name or "default"
  QMap<QString, QString> m_dictionary;  // original word -> replacement
  bool m_markdown_mode = false;
  QString m_style = "Casual";          // Casual | Formal | Email
};

#endif  // SETTING_HPP
