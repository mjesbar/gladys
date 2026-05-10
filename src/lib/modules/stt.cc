#include "stt.h"
#include "keytype.h"
#include <cmath>
#include <cstring>
#include <iostream>
#include <vector>

// Initialize static members
STT *STT::m_instance = nullptr;
ma_device STT::m_audio_device;
ma_device_config STT::m_audio_config;
ma_context STT::m_audio_context;
const SherpaOnnxOnlineRecognizer *STT::m_recognizer = nullptr;
const SherpaOnnxOnlineStream *STT::m_stream = nullptr;

std::string STT::m_encoder_path;
std::string STT::m_decoder_path;
std::string STT::m_joiner_path;
std::string STT::m_tokens_path;
bool STT::m_is_audio_context_initialized = false;
QVector<double> STT::m_audio_levels;
QVector<double> STT::m_audio_levels_prev;
QElapsedTimer STT::m_audio_timer;
QByteArray STT::m_audio_buffer;
bool STT::m_is_buffering = false;

STT::STT(QObject *parent) : QObject(parent) {}

STT *STT::instance() {
  if (!m_instance) {
    m_instance = new STT();
  }
  return m_instance;
}

QVector<double> STT::getAudioLevels() {
  return m_audio_levels;
}

QByteArray STT::getAudioBuffer() {
  return m_audio_buffer;
}

void STT::clearAudioBuffer() {
  m_audio_buffer.clear();
}

void STT::setBuffering(bool enable) {
  std::cout << "STT::setBuffering(" << enable << ") called, current buffer size: " << m_audio_buffer.size() << std::endl;
  m_is_buffering = enable;
  // Don't clear buffer here - we need it for LLM processing!
}

STT::~STT() {
  if (m_recognizer) {
    SherpaOnnxDestroyOnlineRecognizer(m_recognizer);
    m_recognizer = nullptr;
  }
  if (ma_device_is_started(&m_audio_device)) {
    ma_device_uninit(&m_audio_device);
  }
  if (m_is_audio_context_initialized) {
    ma_context_uninit(&m_audio_context);
    m_is_audio_context_initialized = false;
  }
}

bool STT::load(const std::string &model_path) {
  instance();

  std::cout << "STT: Loading model from: " << model_path << std::endl;

  SherpaOnnxOnlineRecognizerConfig config;
  memset(&config, 0, sizeof(config));

  m_encoder_path = model_path + "/encoder.onnx";
  m_decoder_path = model_path + "/decoder.onnx";
  m_joiner_path = model_path + "/joiner.onnx";
  m_tokens_path = model_path + "/tokens.txt";

  if (!SherpaOnnxFileExists(m_encoder_path.c_str())) {
    std::cerr << "STT: Encoder model not found: " << m_encoder_path << std::endl;
    return false;
  }
  if (!SherpaOnnxFileExists(m_decoder_path.c_str())) {
    std::cerr << "STT: Decoder model not found: " << m_decoder_path << std::endl;
    return false;
  }
  if (!SherpaOnnxFileExists(m_joiner_path.c_str())) {
    std::cerr << "STT: Joiner model not found: " << m_joiner_path << std::endl;
    return false;
  }
  if (!SherpaOnnxFileExists(m_tokens_path.c_str())) {
    std::cerr << "STT: Tokens file not found: " << m_tokens_path << std::endl;
    return false;
  }

  config.model_config.transducer.encoder = m_encoder_path.c_str();
  config.model_config.transducer.decoder = m_decoder_path.c_str();
  config.model_config.transducer.joiner = m_joiner_path.c_str();
  config.model_config.tokens = m_tokens_path.c_str();
  config.model_config.num_threads = 1;
  config.model_config.provider = "cpu";
  config.decoding_method = "greedy_search";
  config.feat_config.sample_rate = 16000;
  config.feat_config.feature_dim = 80;

  m_recognizer = SherpaOnnxCreateOnlineRecognizer(&config);
  if (!m_recognizer) {
    std::cerr << "STT: Failed to create recognizer." << std::endl;
    return false;
  }

  ma_result result = ma_context_init(NULL, 0, NULL, &m_audio_context);
  if (result != MA_SUCCESS) {
    std::cerr << "STT: Failed to initialize miniaudio context." << std::endl;
    return false;
  }
  m_is_audio_context_initialized = true;

  m_audio_config = ma_device_config_init(ma_device_type_capture);
  m_audio_config.capture.format = ma_format_s16;
  m_audio_config.capture.channels = 1;
  m_audio_config.sampleRate = 16000;
  m_audio_config.dataCallback = data_callback;
  m_audio_config.pUserData = m_instance;

  result = ma_device_init(&m_audio_context, &m_audio_config, &m_audio_device);
  if (result != MA_SUCCESS) {
    std::cerr << "STT: Failed to initialize miniaudio device." << std::endl;
    ma_context_uninit(&m_audio_context);
    return false;
  }

  std::cout << "STT: Model loaded and audio device initialized." << std::endl;
  return true;
}

void STT::start() {
  if (m_instance == nullptr || !m_recognizer) {
    std::cerr << "STT: Module not loaded. Call load() first." << std::endl;
    return;
  }

  if (ma_device_is_started(&m_audio_device)) {
    std::cout << "STT: Audio device already started." << std::endl;
    return;
  }

  ma_result result = ma_device_start(&m_audio_device);
  if (result != MA_SUCCESS) {
    std::cerr << "STT: Failed to start audio device." << std::endl;
    return;
  }

  m_stream = SherpaOnnxCreateOnlineStream(m_recognizer);
  if (!m_stream) {
    std::cerr << "STT: Failed to create stream." << std::endl;
    return;
  }

  std::cout << "STT: Audio recording started." << std::endl;
}

void STT::stop() {
  if (m_instance == nullptr || !m_recognizer) {
    return;
  }

  if (!ma_device_is_started(&m_audio_device)) {
    return;
  }

  ma_device_stop(&m_audio_device);
  std::cout << "STT: Audio recording stopped." << std::endl;

  if (m_stream) {
    SherpaOnnxOnlineStreamInputFinished(m_stream);
    while (SherpaOnnxIsOnlineStreamReady(m_recognizer, m_stream)) {
      SherpaOnnxDecodeOnlineStream(m_recognizer, m_stream);
    }
    const SherpaOnnxOnlineRecognizerResult *result =
        SherpaOnnxGetOnlineStreamResult(m_recognizer, m_stream);
    if (result && result->text[0] != '\0') {
      std::cout << "Final Transcription: " << result->text << std::endl;
      KeyType::instance()->push(QString::fromUtf8(result->text));
    }
    SherpaOnnxDestroyOnlineRecognizerResult(result);

    SherpaOnnxDestroyOnlineStream(m_stream);
    m_stream = nullptr;
  }

  KeyType::instance()->reset();
}

void STT::data_callback(ma_device *pDevice, void *pOutput, const void *pInput,
                        ma_uint32 frameCount) {
  STT *stt_instance = static_cast<STT *>(pDevice->pUserData);

  if (stt_instance == nullptr || stt_instance->m_recognizer == nullptr ||
      stt_instance->m_stream == nullptr) {
    return;
  }

  std::vector<float> float_samples(frameCount);
  const int16_t *pcm_samples = static_cast<const int16_t *>(pInput);

  for (ma_uint32 i = 0; i < frameCount; ++i) {
    float_samples[i] = static_cast<float>(pcm_samples[i]) / 32768.0f;
  }

  // Audio levels for visualization
  int samplesPerBar = frameCount / 30;
  if (samplesPerBar < 1) samplesPerBar = 1;

  m_audio_levels.clear();
  for (int i = 0; i < 30; ++i) {
    int sampleIdx = i * samplesPerBar;
    if (sampleIdx >= static_cast<int>(frameCount)) sampleIdx = frameCount - 1;
    float sample = std::abs(pcm_samples[sampleIdx]) / 32768.0f;
    double level = qMin(1.0, sample * 15.0);
    m_audio_levels.append(level);
  }

  // Smooth the buffer
  if (m_audio_levels.size() == 30 && m_audio_levels_prev.size() == 30) {
    for (int i = 0; i < 30; ++i) {
      m_audio_levels[i] = m_audio_levels_prev[i] * 0.7 + m_audio_levels[i] * 0.3;
    }
  }
  m_audio_levels_prev = m_audio_levels;

  // Buffer raw PCM audio for LLM processing
  if (m_is_buffering) {
    m_audio_buffer.append(reinterpret_cast<const char *>(pcm_samples), frameCount * sizeof(int16_t));
  }

  // Throttle audio level updates to ~30fps directly in the callback
  if (!m_audio_timer.isValid()) m_audio_timer.start();
  if (m_audio_timer.hasExpired(33)) {
    m_audio_timer.restart();
    emit stt_instance->audioLevelUpdated(m_audio_levels);
  }

  SherpaOnnxOnlineStreamAcceptWaveform(stt_instance->m_stream,
                                       stt_instance->m_audio_config.sampleRate,
                                       float_samples.data(), frameCount);

  if (SherpaOnnxIsOnlineStreamReady(stt_instance->m_recognizer,
                                    stt_instance->m_stream)) {
    SherpaOnnxDecodeOnlineStream(stt_instance->m_recognizer,
                                 stt_instance->m_stream);
    const SherpaOnnxOnlineRecognizerResult *result =
        SherpaOnnxGetOnlineStreamResult(stt_instance->m_recognizer,
                                        stt_instance->m_stream);
    if (result && result->text[0] != '\0') {
      std::cout << "Transcription: " << result->text << std::endl;
      emit m_instance->textReceived(QString::fromUtf8(result->text));
    }
    SherpaOnnxDestroyOnlineRecognizerResult(result);
  }
}