// Sound Menu - Capture input source and the device list (miniaudio).

#ifndef SETTING_SOUND_HPP
#define SETTING_SOUND_HPP

#include <QStringList>
#include <QVariantMap>

class Sound {
 public:
  static QStringList input_sources();  // "default" plus capture devices
  static QStringList available_sfx();  // dropdown entries: every sfx except end.mp3

  QVariantMap to_map() const;
  void from_map(const QVariantMap& values);  // unknown input sources and sfx are ignored

 private:
  QString m_input_source = "default";  // capture device name or "default"
  bool m_sound_effects = true;         // false: no sfx on toggle
  QString m_sound_fx = "switcher";     // sfx played on toggle on, from available_sfx()
};

#endif  // SETTING_SOUND_HPP
