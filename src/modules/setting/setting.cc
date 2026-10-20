// Setting Module - Persistent user preferences (~/.config/gladys/gladys.ini).

#include "setting.hpp"

#include <QSettings>

#include "../log/log.hpp"

// Miniaudio header only (implementation is in stt.cc)
#include "miniaudio/miniaudio.h"

Setting* Setting::s_instance = nullptr;

Setting* Setting::instance() {
  if (!s_instance) {
    s_instance = new Setting();
  }
  return s_instance;
}

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

void Setting::save() {
  QSettings s("gladys", "gladys");
  s.setValue("audio/inputSource", m_input_source);

  write_pairs(s, "dictionary", m_dictionary);
  write_pairs(s, "snippets", m_snippets);

  s.setValue("style/markdownMode", m_markdown_mode);
  s.setValue("style/tone", m_style);

  emit SETTINGS_CHANGED(m_input_source);
}

QStringList Setting::available_styles() { return {"Casual", "Formal", "Email"}; }

QStringList Setting::available_input_sources() {
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

QString Setting::input_source() const { return m_input_source; }
QMap<QString, QString> Setting::dictionary() const { return m_dictionary; }
QMap<QString, QString> Setting::snippets() const { return m_snippets; }
bool Setting::markdown_mode() const { return m_markdown_mode; }
QString Setting::style() const { return m_style; }

void Setting::set_input_source(const QString& v) { m_input_source = v; }
void Setting::set_dictionary(const QMap<QString, QString>& d) { m_dictionary = d; }
void Setting::set_snippets(const QMap<QString, QString>& d) { m_snippets = d; }
void Setting::set_markdown_mode(bool v) { m_markdown_mode = v; }
void Setting::set_style(const QString& v) { m_style = v; }
