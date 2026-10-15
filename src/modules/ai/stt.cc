// STT Module - Speech-to-Text using Sherpa-ONNX. Runs on its own QThread.

#include "stt.hpp"

#include <cmath>
#include <cstdio>
#include <cstring>
#include <vector>

#include "../setting/setting.hpp"

static const int SAMPLE_RATE = 16000;
static const int LEVEL_BARS = 30;

STT::STT()
    : m_is_context_ready(false), m_is_device_ready(false), m_recognizer(nullptr),
      m_stream(nullptr) {
  memset(&m_context, 0, sizeof(m_context));
  memset(&m_device, 0, sizeof(m_device));
}

STT::~STT() {
  if (m_stream) {
    SherpaOnnxDestroyOnlineStream(m_stream);
  }
  if (m_is_device_ready) {
    ma_device_uninit(&m_device);
  }
  if (m_is_context_ready) {
    ma_context_uninit(&m_context);
  }
  if (m_recognizer) {
    SherpaOnnxDestroyOnlineRecognizer(m_recognizer);
  }
}

bool STT::init(const std::string &model_dir) {
  std::string encoder = model_dir + "/encoder.onnx";
  std::string decoder = model_dir + "/decoder.onnx";
  std::string joiner = model_dir + "/joiner.onnx";
  std::string tokens = model_dir + "/tokens.txt";

  for (const std::string *path : {&encoder, &decoder, &joiner, &tokens}) {
    if (!SherpaOnnxFileExists(path->c_str())) {
      fprintf(stderr, "STT: model file not found: %s\n", path->c_str());
      return false;
    }
  }

  SherpaOnnxOnlineRecognizerConfig config;
  memset(&config, 0, sizeof(config));
  config.model_config.transducer.encoder = encoder.c_str();
  config.model_config.transducer.decoder = decoder.c_str();
  config.model_config.transducer.joiner = joiner.c_str();
  config.model_config.tokens = tokens.c_str();
  config.model_config.num_threads = 1;
  config.model_config.provider = "cpu";
  config.decoding_method = "greedy_search";
  config.feat_config.sample_rate = SAMPLE_RATE;
  config.feat_config.feature_dim = 80;

  m_recognizer = SherpaOnnxCreateOnlineRecognizer(&config);
  if (!m_recognizer) {
    fprintf(stderr, "STT: failed to create recognizer\n");
    return false;
  }

  if (ma_context_init(NULL, 0, NULL, &m_context) != MA_SUCCESS) {
    fprintf(stderr, "STT: failed to initialize audio context\n");
    return false;
  }
  m_is_context_ready = true;

  return open_device(Setting::instance()->input_source());
}

// Selects the capture device by name ("default" = system default) and
// (re)initializes the device. Must not be called while the device is running.
bool STT::open_device(const QString &source) {
  if (m_is_device_ready) {
    ma_device_uninit(&m_device);
    m_is_device_ready = false;
  }

  m_config = ma_device_config_init(ma_device_type_capture);
  m_config.capture.format = ma_format_s16;
  m_config.capture.channels = 1;
  m_config.sampleRate = SAMPLE_RATE;
  m_config.dataCallback = data_callback;
  m_config.pUserData = this;
  m_config.capture.pDeviceID = NULL;

  if (source != QStringLiteral("default")) {
    ma_device_info *pInfos;
    ma_uint32 count;
    if (ma_context_get_devices(&m_context, NULL, NULL, &pInfos, &count) == MA_SUCCESS) {
      for (ma_uint32 i = 0; i < count; ++i) {
        if (QString::fromUtf8(pInfos[i].name) == source) {
          m_device_id = pInfos[i].id;
          m_config.capture.pDeviceID = &m_device_id;
          break;
        }
      }
    }
  }

  if (ma_device_init(&m_context, &m_config, &m_device) != MA_SUCCESS) {
    fprintf(stderr, "STT: failed to initialize capture device: %s\n",
            qPrintable(source));
    return false;
  }
  m_is_device_ready = true;
  return true;
}

void STT::start_recording() {
  if (!m_recognizer || !m_is_device_ready || ma_device_is_started(&m_device)) {
    return;
  }

  // Create the stream before the device starts, so the callback never races it.
  m_stream = SherpaOnnxCreateOnlineStream(m_recognizer);
  if (!m_stream) {
    fprintf(stderr, "STT: failed to create stream\n");
    return;
  }
  if (ma_device_start(&m_device) != MA_SUCCESS) {
    fprintf(stderr, "STT: failed to start capture device\n");
    SherpaOnnxDestroyOnlineStream(m_stream);
    m_stream = nullptr;
  }
}

void STT::stop_recording() {
  if (!m_is_device_ready || !ma_device_is_started(&m_device)) {
    return;
  }
  ma_device_stop(&m_device); // joins the audio callback before we touch m_stream
  if (!m_stream) {
    return;
  }

  SherpaOnnxOnlineStreamInputFinished(m_stream);
  while (SherpaOnnxIsOnlineStreamReady(m_recognizer, m_stream)) {
    SherpaOnnxDecodeOnlineStream(m_recognizer, m_stream);
  }
  const SherpaOnnxOnlineRecognizerResult *result =
      SherpaOnnxGetOnlineStreamResult(m_recognizer, m_stream);
  if (result && result->text[0] != '\0') {
    emit FINAL_TEXT_READY(QString::fromUtf8(result->text));
  }
  SherpaOnnxDestroyOnlineRecognizerResult(result);

  SherpaOnnxDestroyOnlineStream(m_stream);
  m_stream = nullptr;
}

void STT::reconfigure_audio_source(const QString &source) {
  if (!m_is_context_ready) {
    return;
  }
  bool wasRunning = m_is_device_ready && ma_device_is_started(&m_device);
  if (wasRunning) {
    stop_recording();
  }
  if (!open_device(source)) {
    return;
  }
  if (wasRunning) {
    start_recording();
  }
}

void STT::data_callback(ma_device *pDevice, void *pOutput, const void *pInput,
                        ma_uint32 frameCount) {
  Q_UNUSED(pOutput);
  STT *self = static_cast<STT *>(pDevice->pUserData);
  if (!self->m_stream) {
    return;
  }

  const int16_t *pcm = static_cast<const int16_t *>(pInput);
  std::vector<float> samples(frameCount);
  for (ma_uint32 i = 0; i < frameCount; ++i) {
    samples[i] = static_cast<float>(pcm[i]) / 32768.0f;
  }

  // Audio levels for the wave bars, smoothed against the previous frame
  int samplesPerBar = qMax(1, static_cast<int>(frameCount) / LEVEL_BARS);
  self->m_audio_levels.clear();
  for (int i = 0; i < LEVEL_BARS; ++i) {
    int idx = qMin(i * samplesPerBar, static_cast<int>(frameCount) - 1);
    double sample = std::abs(pcm[idx]) / 32768.0;
    self->m_audio_levels.append(qMin(1.0, sample * 15.0));
  }
  if (self->m_audio_levels_prev.size() == LEVEL_BARS) {
    for (int i = 0; i < LEVEL_BARS; ++i) {
      self->m_audio_levels[i] =
          self->m_audio_levels_prev[i] * 0.7 + self->m_audio_levels[i] * 0.3;
    }
  }
  self->m_audio_levels_prev = self->m_audio_levels;

  if (!self->m_audio_timer.isValid()) {
    self->m_audio_timer.start();
  }
  if (self->m_audio_timer.hasExpired(33)) { // ~30fps
    self->m_audio_timer.restart();
    emit self->AUDIO_LEVEL_UPDATED(self->m_audio_levels);
  }

  SherpaOnnxOnlineStreamAcceptWaveform(self->m_stream, SAMPLE_RATE,
                                       samples.data(), frameCount);
  if (SherpaOnnxIsOnlineStreamReady(self->m_recognizer, self->m_stream)) {
    SherpaOnnxDecodeOnlineStream(self->m_recognizer, self->m_stream);
  }
}
