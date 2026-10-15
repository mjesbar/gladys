// Settings Manager - Singleton managing persistent user preferences.

#include "settings_manager.hpp"

#include <QCoreApplication>
#include <QDir>
#include <QMap>
#include <QStringList>

// Miniaudio header only (implementation is in stt.cc)
#include "miniaudio/miniaudio.h"

// ---------------------------------------------------------------------------
// Static members
// ---------------------------------------------------------------------------
SettingsManager *SettingsManager::s_instance = nullptr;

SettingsManager *SettingsManager::instance() {
  if (!s_instance) {
    s_instance = new SettingsManager();
    s_instance->load();
  }
  return s_instance;
}

SettingsManager::SettingsManager(QObject *parent) : QObject(parent) {}

SettingsManager::~SettingsManager() { save(); }

void SettingsManager::load() {
  QSettings s("gladys", "gladys");
  apply_from_settings(s);
}

void SettingsManager::save() {
  QSettings s("gladys", "gladys");
  write_to_settings(s);
  emit settings_changed();
}

void SettingsManager::reset_to_defaults() {
  m_data = SettingsData();
  save();
}

void SettingsManager::set_input_source(const QString &v) {
  m_data.input_source = v;
  save();
}

void SettingsManager::set_dictionary(const QMap<QString, QString> &d) {
  m_data.dictionary = d;
  save();
}

void SettingsManager::set_markdown_mode(bool v) {
  m_data.markdown_mode = v;
  save();
}

void SettingsManager::set_allow_accents(bool v) {
  m_data.allow_accents = v;
  save();
}

void SettingsManager::set_allow_punctuation(bool v) {
  m_data.allow_punctuation = v;
  save();
}

void SettingsManager::apply_from_settings(QSettings &s) {
  m_data.input_source =
      s.value("audio/inputSource", default_input_source()).toString();

  m_data.dictionary.clear();
  int dictSize = s.beginReadArray("dictionary");
  for (int i = 0; i < dictSize; ++i) {
    s.setArrayIndex(i);
    QString from = s.value("from").toString();
    QString to = s.value("to").toString();
    if (!from.isEmpty()) {
      m_data.dictionary.insert(from, to);
    }
  }
  s.endArray();

  m_data.markdown_mode = s.value("style/markdownMode", false).toBool();
  m_data.allow_accents = s.value("style/allowAccents", true).toBool();
  m_data.allow_punctuation = s.value("style/allowPunctuation", true).toBool();
}

void SettingsManager::write_to_settings(QSettings &s) const {
  s.setValue("audio/inputSource", m_data.input_source);

  s.beginWriteArray("dictionary");
  int idx = 0;
  for (auto it = m_data.dictionary.cbegin(); it != m_data.dictionary.cend();
       ++it) {
    s.setArrayIndex(idx++);
    s.setValue("from", it.key());
    s.setValue("to", it.value());
  }
  s.endArray();

  s.setValue("style/markdownMode", m_data.markdown_mode);
  s.setValue("style/allowAccents", m_data.allow_accents);
  s.setValue("style/allowPunctuation", m_data.allow_punctuation);
}

// ---------------------------------------------------------------------------
// Platform-specific input source enumeration
// ---------------------------------------------------------------------------
QStringList SettingsManager::available_input_sources() {
  QStringList sources;
  sources << "default";

  ma_context context;
  ma_device_info *pCaptureInfos;
  ma_uint32 captureCount;

  ma_result result = ma_context_init(NULL, 0, NULL, &context);
  if (result != MA_SUCCESS) {
    return sources;
  }

  result = ma_context_get_devices(&context, NULL, NULL, &pCaptureInfos,
                                  &captureCount);
  if (result == MA_SUCCESS) {
    for (ma_uint32 i = 0; i < captureCount; ++i) {
      sources << QString::fromUtf8(pCaptureInfos[i].name);
    }
  }

  ma_context_uninit(&context);
  return sources;
}

QString SettingsManager::default_input_source() {
  return QStringLiteral("default");
}

QString SettingsManager::input_source() const { return m_data.input_source; }
QMap<QString, QString> SettingsManager::dictionary() const { return m_data.dictionary; }
bool SettingsManager::markdown_mode() const { return m_data.markdown_mode; }
bool SettingsManager::allow_accents() const { return m_data.allow_accents; }
bool SettingsManager::allow_punctuation() const { return m_data.allow_punctuation; }
SettingsData SettingsManager::data() const { return m_data; }
