// STT - Speech-to-Text using Sherpa-ONNX. Runs inside the gladys-asr process.

#include "stt.hpp"

#include <cstring>

#include "../../modules/log/log.hpp"

static const int SAMPLE_RATE = 16000;
static const size_t MAX_SAMPLES = SAMPLE_RATE * 60;  // 60 s cap per utterance

STT::STT() : m_is_context_ready(false), m_is_device_ready(false), m_recognizer(nullptr) {
  memset(&m_context, 0, sizeof(m_context));
  memset(&m_device, 0, sizeof(m_device));
}

STT::~STT() {
  qCInfo(lcStt) << "stopped";
  if (m_is_device_ready) {
    ma_device_uninit(&m_device);
  }
  if (m_is_context_ready) {
    ma_context_uninit(&m_context);
  }
  if (m_recognizer) {
    SherpaOnnxDestroyOfflineRecognizer(m_recognizer);
  }
}

bool STT::init(const std::string& model_dir) {
  std::string encoder = model_dir + "/encoder.int8.onnx";
  std::string decoder = model_dir + "/decoder.int8.onnx";
  std::string joiner = model_dir + "/joiner.int8.onnx";
  std::string tokens = model_dir + "/tokens.txt";

  for (const std::string* path : {&encoder, &decoder, &joiner, &tokens}) {
    if (!SherpaOnnxFileExists(path->c_str())) {
      qCCritical(lcStt) << "model file not found:" << path->c_str();
      return false;
    }
  }

  SherpaOnnxOfflineRecognizerConfig config;
  memset(&config, 0, sizeof(config));
  config.model_config.transducer.encoder = encoder.c_str();
  config.model_config.transducer.decoder = decoder.c_str();
  config.model_config.transducer.joiner = joiner.c_str();
  config.model_config.tokens = tokens.c_str();
  config.model_config.model_type = "nemo_transducer";
  config.model_config.num_threads = 1;
  config.model_config.provider = "cpu";
  config.decoding_method = "greedy_search";
  config.feat_config.sample_rate = SAMPLE_RATE;
  config.feat_config.feature_dim = 80;

  m_recognizer = SherpaOnnxCreateOfflineRecognizer(&config);
  if (!m_recognizer) {
    qCCritical(lcStt) << "failed to create recognizer";
    return false;
  }

  if (ma_context_init(NULL, 0, NULL, &m_context) != MA_SUCCESS) {
    qCCritical(lcStt) << "failed to initialize audio context";
    return false;
  }
  m_is_context_ready = true;
  return true;  // capture device is opened on the first start
}

// Selects the capture device by name ("default" = system default) and
// (re)initializes the device. Must not be called while the device is running.
bool STT::open_device(const QString& source) {
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
    ma_device_info* pInfos;
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
    qCCritical(lcStt) << "failed to initialize capture device:" << source;
    return false;
  }
  m_is_device_ready = true;
  m_source = source;
  qCInfo(lcStt) << "capture device ready:" << source;
  return true;
}

// The device stays open between utterances; it is reopened only when the source changes.
bool STT::start_recording(const QString& source) {
  if (!m_recognizer || !m_is_context_ready) {
    return false;
  }
  if (m_is_device_ready && ma_device_is_started(&m_device)) {
    return true;
  }
  if (!m_is_device_ready || source != m_source) {
    if (!open_device(source)) {
      return false;
    }
  }

  m_buffer.clear();
  m_buffer.reserve(MAX_SAMPLES);
  if (ma_device_start(&m_device) != MA_SUCCESS) {
    qCCritical(lcStt) << "failed to start capture device";
    return false;
  }
  qCInfo(lcStt) << "audio recognition started";
  return true;
}

QString STT::stop_recording() {
  if (!m_is_device_ready || !ma_device_is_started(&m_device)) {
    return QString();
  }
  ma_device_stop(&m_device);  // joins the audio callback before we touch m_buffer
  qCInfo(lcStt) << "audio recognition stopped";
  if (m_buffer.empty()) {
    return QString();
  }

  const SherpaOnnxOfflineStream* stream = SherpaOnnxCreateOfflineStream(m_recognizer);
  if (!stream) {
    qCCritical(lcStt) << "failed to create stream";
    m_buffer.clear();
    return QString();
  }
  SherpaOnnxAcceptWaveformOffline(
    stream, SAMPLE_RATE, m_buffer.data(), static_cast<int32_t>(m_buffer.size()));
  SherpaOnnxDecodeOfflineStream(m_recognizer, stream);
  const SherpaOnnxOfflineRecognizerResult* result = SherpaOnnxGetOfflineStreamResult(stream);
  QString text = result ? QString::fromUtf8(result->text).trimmed() : QString();
  if (!text.isEmpty()) {
    qCInfo(lcStt) << "transcription output:" << text;
  }
  SherpaOnnxDestroyOfflineRecognizerResult(result);
  SherpaOnnxDestroyOfflineStream(stream);
  m_buffer.clear();
  return text;
}

void STT::data_callback(
  ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount) {
  Q_UNUSED(pOutput);
  STT* self = static_cast<STT*>(pDevice->pUserData);

  const int16_t* pcm = static_cast<const int16_t*>(pInput);
  const size_t room = MAX_SAMPLES - self->m_buffer.size();
  const size_t kept = qMin(static_cast<size_t>(frameCount), room);
  for (size_t i = 0; i < kept; ++i) {
    self->m_buffer.push_back(static_cast<float>(pcm[i]) / 32768.0f);
  }
}
