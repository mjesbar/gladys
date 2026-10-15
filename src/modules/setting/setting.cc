// Setting Module - Persistent user preferences (~/.config/gladys/gladys.ini).

#include "setting.hpp"

#include <QSettings>

// Miniaudio header only (implementation is in stt.cc)
#include "miniaudio/miniaudio.h"

Setting *Setting::s_instance = nullptr;

Setting *Setting::instance() {
  if (!s_instance) {
    s_instance = new Setting();
  }
  return s_instance;
}

Setting::Setting(QObject *parent) : QObject(parent) { load(); }

void Setting::load() {
  QSettings s("gladys", "gladys");
  m_input_source = s.value("audio/inputSource", "default").toString();

  m_dictionary.clear();
  int dictSize = s.beginReadArray("dictionary");
  for (int i = 0; i < dictSize; ++i) {
    s.setArrayIndex(i);
    QString from = s.value("from").toString();
    if (!from.isEmpty()) {
      m_dictionary.insert(from, s.value("to").toString());
    }
  }
  s.endArray();

  m_markdown_mode = s.value("style/markdownMode", false).toBool();
  m_allow_accents = s.value("style/allowAccents", true).toBool();
  m_allow_punctuation = s.value("style/allowPunctuation", true).toBool();
}

void Setting::save() {
  QSettings s("gladys", "gladys");
  s.setValue("audio/inputSource", m_input_source);

  s.beginWriteArray("dictionary");
  int idx = 0;
  for (auto it = m_dictionary.cbegin(); it != m_dictionary.cend(); ++it) {
    s.setArrayIndex(idx++);
    s.setValue("from", it.key());
    s.setValue("to", it.value());
  }
  s.endArray();

  s.setValue("style/markdownMode", m_markdown_mode);
  s.setValue("style/allowAccents", m_allow_accents);
  s.setValue("style/allowPunctuation", m_allow_punctuation);

  emit SETTINGS_CHANGED(m_input_source);
}

QStringList Setting::available_input_sources() {
  QStringList sources;
  sources << "default";

  ma_context context;
  if (ma_context_init(NULL, 0, NULL, &context) != MA_SUCCESS) {
    return sources;
  }

  ma_device_info *pCaptureInfos;
  ma_uint32 captureCount;
  if (ma_context_get_devices(&context, NULL, NULL, &pCaptureInfos,
                             &captureCount) == MA_SUCCESS) {
    for (ma_uint32 i = 0; i < captureCount; ++i) {
      sources << QString::fromUtf8(pCaptureInfos[i].name);
    }
  }

  ma_context_uninit(&context);
  return sources;
}

QString Setting::input_source() const { return m_input_source; }
QMap<QString, QString> Setting::dictionary() const { return m_dictionary; }
bool Setting::markdown_mode() const { return m_markdown_mode; }
bool Setting::allow_accents() const { return m_allow_accents; }
bool Setting::allow_punctuation() const { return m_allow_punctuation; }

void Setting::set_input_source(const QString &v) { m_input_source = v; }
void Setting::set_dictionary(const QMap<QString, QString> &d) { m_dictionary = d; }
void Setting::set_markdown_mode(bool v) { m_markdown_mode = v; }
void Setting::set_allow_accents(bool v) { m_allow_accents = v; }
void Setting::set_allow_punctuation(bool v) { m_allow_punctuation = v; }
