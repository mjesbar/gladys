#ifndef STT_H
#define STT_H

#include "miniaudio.h"
#include "sherpa-onnx/c-api/c-api.h" // Placeholder, will be resolved by Makefile
#include <QObject>
#include <QVector>
#include <string>

class STT : public QObject {
  Q_OBJECT

public:
  // Delete copy constructor and assignment operator for singleton
  STT(const STT &) = delete;
  STT &operator=(const STT &) = delete;

  static STT *instance();
  static bool load(const std::string &model_path);
  static void start();
  static void stop();
  static QVector<double> getAudioLevels();

private:
  explicit STT(QObject *parent = nullptr); // Private constructor
  ~STT();                                 // Private destructor for cleanup

  static STT *m_instance;
  static ma_device m_audio_device;
  static ma_device_config m_audio_config;
  static ma_context m_audio_context;
  static bool m_is_audio_context_initialized;

  static const SherpaOnnxOnlineRecognizer *m_recognizer;
  static const SherpaOnnxOnlineStream *m_stream;

  static std::string m_encoder_path;
  static std::string m_decoder_path;
  static std::string m_joiner_path;
  static std::string m_tokens_path;
  static QVector<double> m_audio_levels;
  static QVector<double> m_audio_levels_prev;
  static double m_audio_levels_center;

  static void data_callback(ma_device *pDevice, void *pOutput,
                            const void *pInput, ma_uint32 frameCount);

signals:
  void audioLevelUpdated(const QVector<double> &levels);
};

#endif // STT_H
