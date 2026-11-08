// Overlay Bridge - Computes the Overlay window geometry (dot placement, size profile) for QML.

#ifndef UI_OVERLAY_BRIDGE_HPP
#define UI_OVERLAY_BRIDGE_HPP

#include <QObject>
#include <QString>
#include <QVariantMap>

class OverlayBridge : public QObject {
  Q_OBJECT

 public:
  // Dot center in screen pixels: {x, y}. Dot sits 10% of the shorter screen side from each border.
  Q_INVOKABLE QVariantMap dot_center(const QString& position,
    int screen_width,
    int screen_height) const;

  // Window size and dot center inside the window: {width, height, dotX, dotY, expansion}.
  Q_INVOKABLE QVariantMap profile(const QString& size, const QString& position) const;
};

#endif  // UI_OVERLAY_BRIDGE_HPP
