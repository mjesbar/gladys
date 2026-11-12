// Keyboard Menu - Hotkey mode (Toggle or Hold) and the hotkey profile.

#ifndef SETTING_KEYBOARD_HPP
#define SETTING_KEYBOARD_HPP

#include <QStringList>
#include <QVariantMap>

class Keyboard {
 public:
  static QStringList available_modes();     // Toggle | Hold
  static QStringList available_profiles();  // Ctrl+Alt+P | Fn | AltGr

  QVariantMap to_map() const;
  void from_map(const QVariantMap& values);  // unknown modes and profiles are ignored

 private:
  QString m_mode = "Toggle";         // Hold: record only while the hotkey is down
  QString m_profile = "Ctrl+Alt+P";  // trigger key, resolved per OS by the Toggle module
};

#endif  // SETTING_KEYBOARD_HPP
