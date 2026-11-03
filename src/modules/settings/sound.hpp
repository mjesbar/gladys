// Sound Menu - Capture input source and the device list (miniaudio).

#ifndef SETTING_SOUND_HPP
#define SETTING_SOUND_HPP

#include <QStringList>
#include <QVariantMap>

class Sound {
 public:
  static QStringList input_sources();  // "default" plus capture devices

  QVariantMap to_map() const;
  void from_map(const QVariantMap& values);  // unknown input sources are ignored

 private:
  QString m_input_source = "default";  // capture device name or "default"
};

#endif  // SETTING_SOUND_HPP
