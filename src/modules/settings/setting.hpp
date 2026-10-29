// Setting Module - Persistent user preferences (~/.config/gladys/gladys.ini).
// Runs on its own thread: callers reach it with blocking queued calls.

#ifndef SETTING_HPP
#define SETTING_HPP

#include <QMap>
#include <QObject>
#include <QStringList>
#include <QVariantMap>

class Setting : public QObject {
  Q_OBJECT

 public:
  explicit Setting(QObject* parent = nullptr);

  static QStringList available_styles();

 public slots:
  void publish();  // emits SETTINGS_CHANGED with the current values
  QVariantMap snapshot() const;
  QStringList input_sources() const;
  void apply(const QVariantMap& values);  // validates, persists, then publishes

 signals:
  void SETTINGS_CHANGED(const QVariantMap& snapshot);

 private:
  void load();

  QString m_input_source = "default";   // capture device name or "default"
  QMap<QString, QString> m_dictionary;  // rare word -> brief description
  QMap<QString, QString> m_snippets;    // literal text -> replacement (post-LLM)
  bool m_markdown_mode = false;
  QString m_style = "Casual";           // Casual | Formal | Email
};

#endif  // SETTING_HPP
