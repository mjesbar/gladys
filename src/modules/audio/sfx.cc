// Sfx - Plays the toggle sound effects and the Sound FXs previews (miniaudio engine, GUI thread).

#include "sfx.hpp"

#include <QFile>

#include "../log/log.hpp"
#include "../settings/sound.hpp"

static const QString END_SFX = QStringLiteral("end");

Sfx::Sfx(QObject* parent) : QObject(parent) {}

Sfx::~Sfx() {
  stop();
  if (m_is_engine_ready) {
    ma_engine_uninit(&m_engine);
  }
}

void Sfx::init() {
  if (ma_engine_init(NULL, &m_engine) != MA_SUCCESS) {
    qCWarning(lcMain) << "failed to initialize sound effects";
    return;
  }
  m_is_engine_ready = true;
}

void Sfx::update(const QVariantMap& config) {
  m_enabled = config.value("soundEffects").toBool();
  m_fx = config.value("soundFx").toString();
}

void Sfx::play_toggle(bool recording) {
  if (m_enabled) {
    play(recording ? m_fx : END_SFX);
  }
}

void Sfx::preview(const QString& name) {
  if (Sound::available_sfx().contains(name)) {
    play(name);
  }
}

// Sfx are Qt resources, so they are decoded from memory, not from a path.
void Sfx::play(const QString& name) {
  if (!m_is_engine_ready) {
    return;
  }
  stop();

  QFile file(QStringLiteral(":/sfx/%1.mp3").arg(name));
  if (!file.open(QIODevice::ReadOnly)) {
    qCWarning(lcMain) << "missing sound effect" << name;
    return;
  }
  m_data = file.readAll();

  const ma_decoder_config config = ma_decoder_config_init_default();
  if (ma_decoder_init_memory(m_data.constData(), m_data.size(), &config, &m_decoder) != MA_SUCCESS) {
    qCWarning(lcMain) << "failed to decode sound effect" << name;
    m_data.clear();
    return;
  }
  if (ma_sound_init_from_data_source(&m_engine, &m_decoder, 0, NULL, &m_sound) != MA_SUCCESS) {
    qCWarning(lcMain) << "failed to play sound effect" << name;
    ma_decoder_uninit(&m_decoder);
    m_data.clear();
    return;
  }
  m_is_sound_ready = true;
  ma_sound_start(&m_sound);
}

// The sound is uninitialized before the decoder and its buffer it reads from.
void Sfx::stop() {
  if (!m_is_sound_ready) {
    return;
  }
  ma_sound_uninit(&m_sound);
  ma_decoder_uninit(&m_decoder);
  m_data.clear();
  m_is_sound_ready = false;
}
