#include "stt.h"
#include <cstring> // For memset
#include <iostream>
#include <vector>

// Initialize static members
SST *SST::m_instance = nullptr;
ma_device SST::m_audio_device;
ma_device_config SST::m_audio_config;
ma_context SST::m_audio_context;
const SherpaOnnxOnlineRecognizer *SST::m_recognizer = nullptr;
const SherpaOnnxOnlineStream *SST::m_stream = nullptr;

std::string SST::m_encoder_path;
std::string SST::m_decoder_path;
std::string SST::m_joiner_path;
std::string SST::m_tokens_path;
bool SST::m_is_audio_context_initialized = false;

SST::~SST() {
  // Cleanup resources if they were initialized
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

bool SST::load(const std::string &model_path) {
  if (m_instance == nullptr) {
    m_instance = new SST();
  }

  std::cout << "SST: Loading STT model from: " << model_path << std::endl;

  // Sherpa-ONNX model loading
  SherpaOnnxOnlineRecognizerConfig config;
  memset(&config, 0, sizeof(config));

  // Assuming a streaming zipformer model
  m_encoder_path = model_path + "/encoder.onnx";
  m_decoder_path = model_path + "/decoder.onnx";
  m_joiner_path = model_path + "/joiner.onnx";
  m_tokens_path = model_path + "/tokens.txt";

  std::cout << "SST: Verifying model files:" << std::endl;
  if (!SherpaOnnxFileExists(m_encoder_path.c_str())) {
    std::cerr << "SST: Encoder model not found: " << m_encoder_path
              << std::endl;
    return false;
  }
  std::cout << "SST: Encoder: " << m_encoder_path << std::endl;

  if (!SherpaOnnxFileExists(m_decoder_path.c_str())) {
    std::cerr << "SST: Decoder model not found: " << m_decoder_path
              << std::endl;
    return false;
  }
  std::cout << "SST: Decoder: " << m_decoder_path << std::endl;

  if (!SherpaOnnxFileExists(m_joiner_path.c_str())) {
    std::cerr << "SST: Joiner model not found: " << m_joiner_path << std::endl;
    return false;
  }
  std::cout << "SST: Joiner: " << m_joiner_path << std::endl;

  if (!SherpaOnnxFileExists(m_tokens_path.c_str())) {
    std::cerr << "SST: Tokens file not found: " << m_tokens_path << std::endl;
    return false;
  }
  std::cout << "SST: Tokens: " << m_tokens_path << std::endl;

  config.model_config.transducer.encoder = m_encoder_path.c_str();
  config.model_config.transducer.decoder = m_decoder_path.c_str();
  config.model_config.transducer.joiner = m_joiner_path.c_str();
  config.model_config.tokens = m_tokens_path.c_str();

  config.model_config.num_threads = 1; // Explicitly set number of threads

  config.model_config.provider = "cpu"; // Optionally "cpu"

  config.decoding_method = "greedy_search";
  config.feat_config.sample_rate = 16000; // Assuming 16kHz
  config.feat_config.feature_dim = 80;    // Common value for ASR models

  std::cout << "SST: Calling SherpaOnnxCreateOnlineRecognizer..." << std::endl;
  m_recognizer = SherpaOnnxCreateOnlineRecognizer(&config);
  std::cout << "SST: SherpaOnnxCreateOnlineRecognizer returned." << std::endl;

  if (!m_recognizer) {
    std::cerr << "SST: Failed to create SherpaOnnxOnlineRecognizer."
              << std::endl;
    return false;
  }

  // MiniAudio initialization
  ma_result result;

  result = ma_context_init(NULL, 0, NULL, &m_audio_context);
  if (result != MA_SUCCESS) {
    std::cerr << "SST: Failed to initialize miniaudio context." << std::endl;
    return false;
  }
  m_is_audio_context_initialized = true;

  m_audio_config = ma_device_config_init(ma_device_type_capture);
  m_audio_config.capture.format = ma_format_s16; // 16-bit signed integer
  m_audio_config.capture.channels = 1;           // Mono
  m_audio_config.sampleRate = 16000;             // 16kHz sample rate
  m_audio_config.dataCallback = data_callback;
  m_audio_config.pUserData =
      m_instance; // Pass the SST instance to the callback

  result = ma_device_init(&m_audio_context, &m_audio_config, &m_audio_device);
  if (result != MA_SUCCESS) {
    std::cerr << "SST: Failed to initialize miniaudio device." << std::endl;
    ma_context_uninit(&m_audio_context);
    return false;
  }

  std::cout << "SST: Model loaded and audio device initialized." << std::endl;
  return true;
}

void SST::start() {
  if (m_instance == nullptr || !m_recognizer) {
    std::cerr << "SST: Module not loaded. Call load() first." << std::endl;
    return;
  }

  if (ma_device_is_started(&m_audio_device)) {
    std::cout << "SST: Audio device already started." << std::endl;
    return;
  }

  ma_result result = ma_device_start(&m_audio_device);
  if (result != MA_SUCCESS) {
    std::cerr << "SST: Failed to start audio device." << std::endl;
    return;
  }

  m_stream = SherpaOnnxCreateOnlineStream(m_recognizer);
  if (!m_stream) {
    std::cerr << "SST: Failed to create SherpaOnnxOnlineStream." << std::endl;
    return;
  }

  std::cout << "SST: Audio recording started." << std::endl;
}

void SST::stop() {
  if (m_instance == nullptr || !m_recognizer) {
    std::cerr << "SST: Module not loaded. Call load() first." << std::endl;
    return;
  }

  if (!ma_device_is_started(&m_audio_device)) {
    std::cout << "SST: Audio device already stopped." << std::endl;
    return;
  }

  ma_device_stop(&m_audio_device);
  std::cout << "SST: Audio recording stopped." << std::endl;

  if (m_stream) {
    // Finalize the stream and get any remaining transcription
    SherpaOnnxOnlineStreamInputFinished(m_stream); // Signal end of input
    while (SherpaOnnxIsOnlineStreamReady(m_recognizer, m_stream)) {
      SherpaOnnxDecodeOnlineStream(m_recognizer, m_stream);
    }
    const SherpaOnnxOnlineRecognizerResult *result =
        SherpaOnnxGetOnlineStreamResult(m_recognizer, m_stream);
    if (result && result->text[0] != '\0') {
      std::cout << "Final Transcription: " << result->text << std::endl;
    }
    SherpaOnnxDestroyOnlineRecognizerResult(result);

    SherpaOnnxDestroyOnlineStream(m_stream);
    m_stream = nullptr;
  }
}

void SST::data_callback(ma_device *pDevice, void *pOutput, const void *pInput,
                        ma_uint32 frameCount) {
  // Cast pUserData back to SST instance
  SST *sst_instance = static_cast<SST *>(pDevice->pUserData);

  if (sst_instance == nullptr || sst_instance->m_recognizer == nullptr ||
      sst_instance->m_stream == nullptr) {
    return; // Not initialized or started yet
  }

  // Feed audio to sherpa-onnx
  // Assuming 16-bit signed integer samples, convert to float
  std::vector<float> float_samples(frameCount);
  const int16_t *pcm_samples = static_cast<const int16_t *>(pInput);
  for (ma_uint32 i = 0; i < frameCount; ++i) {
    float_samples[i] = static_cast<float>(pcm_samples[i]) / 32768.0f;
  }
  SherpaOnnxOnlineStreamAcceptWaveform(sst_instance->m_stream,
                                       sst_instance->m_audio_config.sampleRate,
                                       float_samples.data(), frameCount);

  // Decode and get results
  if (SherpaOnnxIsOnlineStreamReady(sst_instance->m_recognizer,
                                    sst_instance->m_stream)) {
    SherpaOnnxDecodeOnlineStream(sst_instance->m_recognizer,
                                 sst_instance->m_stream);
    const SherpaOnnxOnlineRecognizerResult *result =
        SherpaOnnxGetOnlineStreamResult(sst_instance->m_recognizer,
                                        sst_instance->m_stream);
    if (result && result->text[0] != '\0') {
      std::cout << "Transcription: " << result->text << std::endl;
    }
    SherpaOnnxDestroyOnlineRecognizerResult(result);
  }
}
