// Settings Manager - Singleton managing persistent user preferences.

#include "settings_manager.h"

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
  applyFromSettings(s);
}

void SettingsManager::save() {
  QSettings s("gladys", "gladys");
  writeToSettings(s);
  emit settingsChanged();
}

void SettingsManager::resetToDefaults() {
  m_data = SettingsData();
  save();
}

void SettingsManager::setInputSource(const QString &v) {
  m_data.inputSource = v;
  save();
}

void SettingsManager::setDictionary(const QMap<QString, QString> &d) {
  m_data.dictionary = d;
  save();
}

void SettingsManager::setMarkdownMode(bool v) {
  m_data.markdownMode = v;
  save();
}

void SettingsManager::setAllowAccents(bool v) {
  m_data.allowAccents = v;
  save();
}

void SettingsManager::setAllowPunctuation(bool v) {
  m_data.allowPunctuation = v;
  save();
}

void SettingsManager::applyFromSettings(QSettings &s) {
  m_data.inputSource =
      s.value("audio/inputSource", defaultInputSource()).toString();

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

  m_data.markdownMode = s.value("style/markdownMode", false).toBool();
  m_data.allowAccents = s.value("style/allowAccents", true).toBool();
  m_data.allowPunctuation = s.value("style/allowPunctuation", true).toBool();
}

void SettingsManager::writeToSettings(QSettings &s) const {
  s.setValue("audio/inputSource", m_data.inputSource);

  s.beginWriteArray("dictionary");
  int idx = 0;
  for (auto it = m_data.dictionary.cbegin(); it != m_data.dictionary.cend();
       ++it) {
    s.setArrayIndex(idx++);
    s.setValue("from", it.key());
    s.setValue("to", it.value());
  }
  s.endArray();

  s.setValue("style/markdownMode", m_data.markdownMode);
  s.setValue("style/allowAccents", m_data.allowAccents);
  s.setValue("style/allowPunctuation", m_data.allowPunctuation);
}

// ---------------------------------------------------------------------------
// Platform-specific input source enumeration
// ---------------------------------------------------------------------------
QStringList SettingsManager::availableInputSources() {
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

QString SettingsManager::defaultInputSource() {
  return QStringLiteral("default");
}
