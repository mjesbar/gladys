// Keyboard Menu - Hotkey mode (Toggle or Hold) and the hotkey profile.

#include "keyboard.hpp"

QStringList Keyboard::available_modes() { return {"Toggle", "Hold"}; }

QStringList Keyboard::available_profiles() { return {"Ctrl+Alt+P", "Fn", "AltGr"}; }

QVariantMap Keyboard::to_map() const {
  return {{"hotkeyMode", m_mode}, {"hotkeyProfile", m_profile}};
}

void Keyboard::from_map(const QVariantMap& values) {
  const QString mode = values.value("hotkeyMode", m_mode).toString();
  if (available_modes().contains(mode)) {
    m_mode = mode;
  }
  const QString profile = values.value("hotkeyProfile", m_profile).toString();
  if (available_profiles().contains(profile)) {
    m_profile = profile;
  }
}
