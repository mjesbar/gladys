// Settings Bridge - Exposes the Setting module to the Settings QML window.

#include "settings.hpp"

#include "../setting/setting.hpp"

SettingsBridge::SettingsBridge(QObject* parent) : QObject(parent) {}

QStringList SettingsBridge::input_sources() const { return Setting::available_input_sources(); }

QStringList SettingsBridge::styles() const { return Setting::available_styles(); }

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

QVariantMap SettingsBridge::load() const {
  Setting* s = Setting::instance();
  return {
    {"inputSource", s->input_source()},
    {"markdown", s->markdown_mode()},
    {"style", s->style()},
    {"dictionary", to_variant(s->dictionary())},
    {"snippets", to_variant(s->snippets())},
  };
}

// Unknown input sources and styles are ignored.
void SettingsBridge::apply(const QVariantMap& values) {
  Setting* s = Setting::instance();

  const QString source = values.value("inputSource").toString();
  if (Setting::available_input_sources().contains(source)) {
    s->set_input_source(source);
  }

  const QString style = values.value("style").toString();
  if (Setting::available_styles().contains(style)) {
    s->set_style(style);
  }

  s->set_dictionary(to_pairs(values.value("dictionary").toMap()));
  s->set_snippets(to_pairs(values.value("snippets").toMap()));
  s->set_markdown_mode(values.value("markdown").toBool());
  s->save();
}
