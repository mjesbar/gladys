// Sound Menu - Capture input source and the device list (miniaudio).

#include "sound.hpp"

// Miniaudio is compiled into this executable here, for device enumeration.
#include "miniaudio/miniaudio.h"

QVariantMap Sound::to_map() const {
  return {
    {"inputSource", m_input_source},
    {"soundEffects", m_sound_effects},
    {"soundFx", m_sound_fx},
  };
}

void Sound::from_map(const QVariantMap& values) {
  const QString source = values.value("inputSource", m_input_source).toString();
  if (input_sources().contains(source)) {
    m_input_source = source;
  }
  m_sound_effects = values.value("soundEffects", m_sound_effects).toBool();
  const QString fx = values.value("soundFx", m_sound_fx).toString();
  if (available_sfx().contains(fx)) {
    m_sound_fx = fx;
  }
}

QStringList Sound::available_sfx() {
  return {"beep", "bell", "notification", "pump", "switcher", "ui1", "ui2", "ui3", "ui4", "ui5"};
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
