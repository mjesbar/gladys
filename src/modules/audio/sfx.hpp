// Sfx - Plays the toggle sound effects and the Sound FXs previews (miniaudio engine, GUI thread).

#ifndef SFX_HPP
#define SFX_HPP

#include <QByteArray>
#include <QObject>
#include <QString>
#include <QVariantMap>

#include "miniaudio/miniaudio.h"

class Sfx : public QObject {
  Q_OBJECT

 public:
  explicit Sfx(QObject* parent = nullptr);
  ~Sfx() override;

  void init();  // starts the playback engine; on failure sfx stay silent

 public slots:
  void update(const QVariantMap& config);  // soundEffects and soundFx from settings
  void play_toggle(bool recording);        // selected sfx on start, end.mp3 on stop
  Q_INVOKABLE void preview(const QString& name);  // plays even when effects are off

 private:
  void play(const QString& name);  // stops the sfx still playing, then starts name
  void stop();

  ma_engine m_engine{};
  bool m_is_engine_ready = false;
  ma_decoder m_decoder{};
  ma_sound m_sound{};
  bool m_is_sound_ready = false;
  QByteArray m_data;  // encoded mp3; the decoder reads it, so it lives as long as the sound
  bool m_enabled = true;
  QString m_fx = QStringLiteral("switcher");
};

#endif  // SFX_HPP
