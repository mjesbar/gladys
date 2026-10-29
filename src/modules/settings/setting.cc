// Setting Module - Persistent user preferences (~/.config/gladys/gladys.ini).

#include "setting.hpp"

#include <QSettings>

#include "../log/log.hpp"

// Miniaudio is compiled into this executable here, for device enumeration.
#include "miniaudio/miniaudio.h"

Setting::Setting(QObject* parent) : QObject(parent) { load(); }

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

void Setting::load() {
  QSettings s("gladys", "gladys");
  qCInfo(lcSetting) << "loaded" << s.fileName();
  m_input_source = s.value("audio/inputSource", "default").toString();

  m_dictionary = read_pairs(s, "dictionary");
  m_snippets = read_pairs(s, "snippets");

  m_markdown_mode = s.value("style/markdownMode", false).toBool();
  m_style = s.value("style/tone", "Casual").toString();
  if (!available_styles().contains(m_style)) {
    m_style = "Casual";
  }
}

void Setting::publish() { emit SETTINGS_CHANGED(snapshot()); }

QVariantMap Setting::snapshot() const {
  return {
    {"inputSource", m_input_source},
    {"markdown", m_markdown_mode},
    {"style", m_style},
    {"dictionary", to_variant(m_dictionary)},
    {"snippets", to_variant(m_snippets)},
  };
}

// Unknown input sources and styles are ignored.
void Setting::apply(const QVariantMap& values) {
  const QString source = values.value("inputSource").toString();
  if (input_sources().contains(source)) {
    m_input_source = source;
  }

  const QString style = values.value("style").toString();
  if (available_styles().contains(style)) {
    m_style = style;
  }

  m_dictionary = to_pairs(values.value("dictionary").toMap());
  m_snippets = to_pairs(values.value("snippets").toMap());
  m_markdown_mode = values.value("markdown").toBool();

  QSettings s("gladys", "gladys");
  s.setValue("audio/inputSource", m_input_source);
  write_pairs(s, "dictionary", m_dictionary);
  write_pairs(s, "snippets", m_snippets);
  s.setValue("style/markdownMode", m_markdown_mode);
  s.setValue("style/tone", m_style);

  publish();
}

QStringList Setting::available_styles() { return {"Casual", "Formal", "Email"}; }

QStringList Setting::input_sources() const {
  QStringList sources;
  sources << "default";

  ma_context context;
  if (ma_context_init(NULL, 0, NULL, &context) != MA_SUCCESS) {
    return sources;
  }

  ma_device_info* pCaptureInfos;
  ma_uint32 captureCount;
  if (ma_context_get_devices(&context, NULL, NULL, &pCaptureInfos, &captureCount) == MA_SUCCESS) {
    for (ma_uint32 i = 0; i < captureCount; ++i) {
      sources << QString::fromUtf8(pCaptureInfos[i].name);
    }
  }

  ma_context_uninit(&context);
  return sources;
}
