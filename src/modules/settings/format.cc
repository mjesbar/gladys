// Format Menu - Style, markdown mode, dictionary and snippets.

#include "format.hpp"

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

QVariantMap Format::to_map() const {
  return {
    {"style", m_style},
    {"markdown", m_markdown_mode},
    {"dictionary", to_variant(m_dictionary)},
    {"snippets", to_variant(m_snippets)},
  };
}

// Only keys present in values change: each control saves its own key alone.
void Format::from_map(const QVariantMap& values) {
  if (values.contains("style") && available_styles().contains(values.value("style").toString())) {
    m_style = values.value("style").toString();
  }
  if (values.contains("dictionary")) {
    m_dictionary = to_pairs(values.value("dictionary").toMap());
  }
  if (values.contains("snippets")) {
    m_snippets = to_pairs(values.value("snippets").toMap());
  }
  if (values.contains("markdown")) {
    m_markdown_mode = values.value("markdown").toBool();
  }
}
