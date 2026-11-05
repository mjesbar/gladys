// Appearance Menu - Overlay position, overlay size and the screen it is shown on.

#include "appearance.hpp"

#include <QGuiApplication>
#include <QScreen>

QStringList Appearance::available_positions() {
  return {"up", "right", "down", "left", "up right", "up left", "down right", "down left"};
}

QStringList Appearance::available_sizes() { return {"Minimal", "Basic", "Great"}; }

QStringList Appearance::available_screens() {
  QStringList screens{"Primary"};
  for (const QScreen* screen : QGuiApplication::screens()) {
    screens << screen->name();
  }
  return screens;
}

QVariantMap Appearance::to_map() const {
  return {{"position", m_position}, {"overlaySize", m_overlay_size}, {"screen", m_screen}};
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
  const QString screen = values.value("screen", m_screen).toString();
  if (!screen.isEmpty()) {
    m_screen = screen;
  }
}
