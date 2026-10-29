// Format Menu - Style, markdown mode, dictionary and snippets.

#ifndef SETTING_FORMAT_HPP
#define SETTING_FORMAT_HPP

#include <QMap>
#include <QStringList>
#include <QVariantMap>

class QSettings;

class Format {
 public:
  static QStringList available_styles();  // Casual | Formal | Email

  void load(QSettings& s);
  void save(QSettings& s) const;
  QVariantMap to_map() const;
  void from_map(const QVariantMap& values);  // unknown styles are ignored

 private:
  QString m_style = "Casual";
  bool m_markdown_mode = false;
  QMap<QString, QString> m_dictionary;  // rare word -> brief description
  QMap<QString, QString> m_snippets;    // literal text -> replacement (post-LLM)
};

#endif  // SETTING_FORMAT_HPP
