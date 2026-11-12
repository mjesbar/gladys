// Audio Levels - Live input meter for the wave bars. Runs in the GUI process and
// does not depend on gladys-asr: the bars move even when no transcription runs.

#ifndef LEVELS_HPP
#define LEVELS_HPP

#include <QElapsedTimer>
#include <QObject>
#include <QString>
#include <QVector>

#include "miniaudio/miniaudio.h"

class AudioLevels : public QObject {
  Q_OBJECT

 public:
  explicit AudioLevels(QObject* parent = nullptr);
  ~AudioLevels() override;

  bool init();  // creates the audio context

 public slots:
  void set_source(const QString& source);  // "default" or a capture device name, used on next start
  void set_active(bool active);            // opens and captures while recording

 signals:
  void LEVELS_UPDATED(const QVector<double>& levels);

 private:
  static void data_callback(
    ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount);

  ma_context m_context{};
  ma_device m_device{};
  ma_device_id m_device_id{};
  bool m_is_context_ready = false;
  bool m_is_device_ready = false;
  QString m_source = QStringLiteral("default");
  QVector<double> m_levels_prev;
  QElapsedTimer m_timer;
};

#endif  // LEVELS_HPP
