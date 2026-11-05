// Appearance Menu - Overlay position and overlay size.

#ifndef SETTING_APPEARANCE_HPP
#define SETTING_APPEARANCE_HPP

#include <QStringList>
#include <QVariantMap>

class Appearance {
 public:
  static QStringList available_positions();  // up | right | down | left | up right | up left | down right | down left
  static QStringList available_sizes();      // Minimal | Basic | Great

  QVariantMap to_map() const;
  void from_map(const QVariantMap& values);  // unknown positions and sizes are ignored

 private:
  QString m_position = "up";
  QString m_overlay_size = "Minimal";
};

#endif  // SETTING_APPEARANCE_HPP
