// STT Module - Speech-to-Text using Sherpa-ONNX. Runs on its own QThread.

#ifndef STT_H
#define STT_H

#include "miniaudio/miniaudio.h"
#include "sherpa-onnx/c-api/c-api.h"
#include <QElapsedTimer>
#include <QObject>
#include <QString>
#include <QVector>
#include <string>
#include <vector>

class STT : public QObject {
  Q_OBJECT

public:
  STT();
  ~STT() override;

  bool init(const std::string &model_dir); // call before moveToThread

public slots:
  void start_recording();
  void stop_recording();
  void reconfigure_audio_source(const QString &source);

signals:
  void AUDIO_LEVEL_UPDATED(const QVector<double> &levels);
  void FINAL_TEXT_READY(const QString &text);

private:
  bool open_device(const QString &source);
  static void data_callback(ma_device *pDevice, void *pOutput,
                            const void *pInput, ma_uint32 frameCount);

  ma_context m_context;
  ma_device m_device;
  ma_device_config m_config;
  ma_device_id m_device_id; // m_config.capture.pDeviceID points here
  bool m_is_context_ready;
  bool m_is_device_ready;

  const SherpaOnnxOfflineRecognizer *m_recognizer;
  std::vector<float> m_buffer; // full utterance, decoded once on stop

  QVector<double> m_audio_levels;
  QVector<double> m_audio_levels_prev;
  QElapsedTimer m_audio_timer;
};

#endif // STT_H
