// STT - Speech-to-Text using Sherpa-ONNX. Runs inside the gladys-asr process.

#ifndef STT_H
#define STT_H

#include <QString>
#include <string>
#include <vector>

#include "miniaudio/miniaudio.h"
#include "sherpa-onnx/c-api/c-api.h"

class STT {
 public:
  STT();
  ~STT();

  bool init(const std::string& model_dir);
  bool start_recording(const QString& source);  // "default" or a capture device name
  QString stop_recording();                     // decodes the utterance, "" when silent

 private:
  bool open_device(const QString& source);
  static void data_callback(
    ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount);

  ma_context m_context;
  ma_device m_device;
  ma_device_config m_config;
  ma_device_id m_device_id;  // m_config.capture.pDeviceID points here
  bool m_is_context_ready;
  bool m_is_device_ready;
  QString m_source;  // device currently open

  const SherpaOnnxOfflineRecognizer* m_recognizer;
  std::vector<float> m_buffer;  // full utterance, decoded once on stop
};

#endif  // STT_H
