// Sound Menu - Capture input source and the device list (miniaudio).

#include "sound.hpp"

#include <QSettings>

// Miniaudio is compiled into this executable here, for device enumeration.
#include "miniaudio/miniaudio.h"

void Sound::load(QSettings& s) { m_input_source = s.value("audio/inputSource", "default").toString(); }

void Sound::save(QSettings& s) const { s.setValue("audio/inputSource", m_input_source); }

QVariantMap Sound::to_map() const { return {{"inputSource", m_input_source}}; }

void Sound::from_map(const QVariantMap& values) {
  const QString source = values.value("inputSource").toString();
  if (input_sources().contains(source)) {
    m_input_source = source;
  }
}

QStringList Sound::input_sources() {
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
