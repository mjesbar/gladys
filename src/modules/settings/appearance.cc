// Appearance Menu - Overlay position and overlay size.

#include "appearance.hpp"

QStringList Appearance::available_positions() {
  return {"up", "right", "down", "left", "up right", "up left", "down right", "down left"};
}

QStringList Appearance::available_sizes() { return {"Minimal", "Basic", "Great"}; }

QVariantMap Appearance::to_map() const {
  return {{"position", m_position}, {"overlaySize", m_overlay_size}};
}

void Appearance::from_map(const QVariantMap& values) {
  const QString position = values.value("position", m_position).toString();
  if (available_positions().contains(position)) {
    m_position = position;
  }
  const QString size = values.value("overlaySize", m_overlay_size).toString();
  if (available_sizes().contains(size)) {
    m_overlay_size = size;
  }
}
