// Appearance Menu - Overlay position, overlay size and the screen it is shown on.

#ifndef SETTING_APPEARANCE_HPP
#define SETTING_APPEARANCE_HPP

#include <QStringList>
#include <QVariantMap>

class Appearance {
 public:
  static QStringList available_positions();  // up | right | down | left | up right | up left | down right | down left
  static QStringList available_sizes();      // Minimal | Basic | Great
  static QStringList available_screens();    // Primary | connected screen names (GUI thread only)

  QVariantMap to_map() const;
  void from_map(const QVariantMap& values);  // unknown positions and sizes are ignored; any screen name is kept

 private:
  QString m_position = "down";
  QString m_overlay_size = "Basic";
  QString m_screen = "Primary";  // a disconnected screen keeps its name; the overlay falls back to primary
};

#endif  // SETTING_APPEARANCE_HPP
