// Settings Bridge - Exposes the Setting module to the Settings QML window.

#include "settings_bridge.hpp"

#include "../setting/setting.hpp"

SettingsBridge::SettingsBridge(QObject *parent) : QObject(parent) {}

QStringList SettingsBridge::input_sources() const {
  return Setting::available_input_sources();
}

QVariantMap SettingsBridge::load() const {
  Setting *s = Setting::instance();
  QVariantMap dictionary;
  const auto dict = s->dictionary();
  for (auto it = dict.cbegin(); it != dict.cend(); ++it) {
    dictionary.insert(it.key(), it.value());
  }
  return {
      {"inputSource", s->input_source()},
      {"markdown", s->markdown_mode()},
      {"accents", s->allow_accents()},
      {"punctuation", s->allow_punctuation()},
      {"dictionary", dictionary},
  };
}

// Unknown input sources are ignored; blank dictionary keys are dropped.
void SettingsBridge::apply(const QVariantMap &values) {
  Setting *s = Setting::instance();

  const QString source = values.value("inputSource").toString();
  if (Setting::available_input_sources().contains(source)) {
    s->set_input_source(source);
  }

  QMap<QString, QString> dictionary;
  const QVariantMap entries = values.value("dictionary").toMap();
  for (auto it = entries.cbegin(); it != entries.cend(); ++it) {
    const QString from = it.key().trimmed();
    if (!from.isEmpty()) {
      dictionary.insert(from, it.value().toString().trimmed());
    }
  }
  s->set_dictionary(dictionary);

  s->set_markdown_mode(values.value("markdown").toBool());
  s->set_allow_accents(values.value("accents").toBool());
  s->set_allow_punctuation(values.value("punctuation").toBool());
  s->save();
}
