// Format Menu - Style, markdown mode, dictionary and snippets.

#ifndef SETTING_FORMAT_HPP
#define SETTING_FORMAT_HPP

#include <QMap>
#include <QStringList>
#include <QVariantMap>

class Format {
 public:
  static QStringList available_styles();  // Casual | Formal | Email

  QVariantMap to_map() const;
  void from_map(const QVariantMap& values);  // unknown styles are ignored

 private:
  QString m_style = "Casual";
  bool m_markdown_mode = false;
  bool m_llm = true;                    // false: ASR text is pasted as is
  QMap<QString, QString> m_dictionary;  // rare word -> brief description
  QMap<QString, QString> m_snippets;    // literal text -> replacement (post-LLM)
};

#endif  // SETTING_FORMAT_HPP
