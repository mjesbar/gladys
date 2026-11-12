// Audio Levels - Live input meter for the wave bars. Runs in the GUI process and
// does not depend on gladys-asr: the bars move even when no transcription runs.

#include "levels.hpp"

#include <cmath>

#include "../log/log.hpp"

static const int SAMPLE_RATE = 16000;
static const int LEVEL_BARS = 30;
static const int EMIT_INTERVAL_MS = 33;  // ~30 fps

AudioLevels::AudioLevels(QObject* parent) : QObject(parent) {}

AudioLevels::~AudioLevels() {
  set_active(false);
  if (m_is_context_ready) {
    ma_context_uninit(&m_context);
  }
}

bool AudioLevels::init() {
  if (ma_context_init(NULL, 0, NULL, &m_context) != MA_SUCCESS) {
    qCCritical(lcMain) << "failed to initialize audio context for levels";
    return false;
  }
  m_is_context_ready = true;
  return true;
}

void AudioLevels::set_source(const QString& source) { m_source = source; }

void AudioLevels::set_active(bool active) {
  if (m_is_device_ready) {
    ma_device_uninit(&m_device);
    m_is_device_ready = false;
  }
  if (!active || !m_is_context_ready) {
    return;
  }

  ma_device_config config = ma_device_config_init(ma_device_type_capture);
  config.capture.format = ma_format_s16;
  config.capture.channels = 1;
  config.sampleRate = SAMPLE_RATE;
  config.dataCallback = data_callback;
  config.pUserData = this;

  if (m_source != QStringLiteral("default")) {
    ma_device_info* pInfos;
    ma_uint32 count;
    if (ma_context_get_devices(&m_context, NULL, NULL, &pInfos, &count) == MA_SUCCESS) {
      for (ma_uint32 i = 0; i < count; ++i) {
        if (QString::fromUtf8(pInfos[i].name) == m_source) {
          m_device_id = pInfos[i].id;
          config.capture.pDeviceID = &m_device_id;
          break;
        }
      }
    }
  }

  if (ma_device_init(&m_context, &config, &m_device) != MA_SUCCESS) {
    qCCritical(lcMain) << "failed to open level meter device:" << m_source;
    return;
  }
  m_is_device_ready = true;
  m_levels_prev.clear();
  if (ma_device_start(&m_device) != MA_SUCCESS) {
    qCCritical(lcMain) << "failed to start level meter device";
    set_active(false);
  }
}

// Runs on the audio thread. One sample per bar, scaled and smoothed against the previous frame.
void AudioLevels::data_callback(
  ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount) {
  Q_UNUSED(pOutput);
  if (frameCount == 0) {
    return;
  }
  AudioLevels* self = static_cast<AudioLevels*>(pDevice->pUserData);
  const int16_t* pcm = static_cast<const int16_t*>(pInput);

  const int samplesPerBar = qMax(1, static_cast<int>(frameCount) / LEVEL_BARS);
  QVector<double> levels;
  for (int i = 0; i < LEVEL_BARS; ++i) {
    const int idx = qMin(i * samplesPerBar, static_cast<int>(frameCount) - 1);
    levels.append(qMin(1.0, std::abs(pcm[idx]) / 32768.0 * 15.0));
  }
  if (self->m_levels_prev.size() == LEVEL_BARS) {
    for (int i = 0; i < LEVEL_BARS; ++i) {
      levels[i] = self->m_levels_prev[i] * 0.7 + levels[i] * 0.3;
    }
  }
  self->m_levels_prev = levels;

  if (!self->m_timer.isValid()) {
    self->m_timer.start();
  }
  if (self->m_timer.hasExpired(EMIT_INTERVAL_MS)) {
    self->m_timer.restart();
    emit self->LEVELS_UPDATED(levels);
  }
}
