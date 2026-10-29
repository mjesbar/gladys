// Format Menu - Style, markdown mode, dictionary and snippets.

#include "format.hpp"

#include <QSettings>

static QMap<QString, QString> read_pairs(QSettings& s, const char* group) {
  QMap<QString, QString> pairs;
  int size = s.beginReadArray(group);
  for (int i = 0; i < size; ++i) {
    s.setArrayIndex(i);
    QString from = s.value("from").toString();
    if (!from.isEmpty()) {
      pairs.insert(from, s.value("to").toString());
    }
  }
  s.endArray();
  return pairs;
}

static void write_pairs(QSettings& s, const char* group, const QMap<QString, QString>& pairs) {
  s.beginWriteArray(group);
  int idx = 0;
  for (auto it = pairs.cbegin(); it != pairs.cend(); ++it) {
    s.setArrayIndex(idx++);
    s.setValue("from", it.key());
    s.setValue("to", it.value());
  }
  s.endArray();
}

static QVariantMap to_variant(const QMap<QString, QString>& pairs) {
  QVariantMap map;
  for (auto it = pairs.cbegin(); it != pairs.cend(); ++it) {
    map.insert(it.key(), it.value());
  }
  return map;
}

// Blank keys are dropped.
static QMap<QString, QString> to_pairs(const QVariantMap& map) {
  QMap<QString, QString> pairs;
  for (auto it = map.cbegin(); it != map.cend(); ++it) {
    const QString from = it.key().trimmed();
    if (!from.isEmpty()) {
      pairs.insert(from, it.value().toString().trimmed());
    }
  }
  return pairs;
}

QStringList Format::available_styles() { return {"Casual", "Formal", "Email"}; }

void Format::load(QSettings& s) {
  m_dictionary = read_pairs(s, "dictionary");
  m_snippets = read_pairs(s, "snippets");
  m_markdown_mode = s.value("style/markdownMode", false).toBool();
  m_style = s.value("style/tone", "Casual").toString();
  if (!available_styles().contains(m_style)) {
    m_style = "Casual";
  }
}

void Format::save(QSettings& s) const {
  write_pairs(s, "dictionary", m_dictionary);
  write_pairs(s, "snippets", m_snippets);
  s.setValue("style/markdownMode", m_markdown_mode);
  s.setValue("style/tone", m_style);
}

QVariantMap Format::to_map() const {
  return {
    {"style", m_style},
    {"markdown", m_markdown_mode},
    {"dictionary", to_variant(m_dictionary)},
    {"snippets", to_variant(m_snippets)},
  };
}

void Format::from_map(const QVariantMap& values) {
  const QString style = values.value("style").toString();
  if (available_styles().contains(style)) {
    m_style = style;
  }
  m_dictionary = to_pairs(values.value("dictionary").toMap());
  m_snippets = to_pairs(values.value("snippets").toMap());
  m_markdown_mode = values.value("markdown").toBool();
}
