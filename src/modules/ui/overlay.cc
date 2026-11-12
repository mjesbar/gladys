// Overlay Bridge - Computes the Overlay window geometry (dot placement, size profile) for QML.

#include "overlay.hpp"

#include <QtGlobal>

namespace {

constexpr int GREAT_HALF = 75;  // Great window is 150 x 150: fits the bar row (±60 px) around the mic
constexpr int GREAT_EDGE = 8;   // Great dot center to the window edge it sits on

// Great opens away from its border; diagonals follow the vertical rule.
QString expansion(const QString& position) {
  if (position.startsWith("up")) return "down";
  if (position.startsWith("down")) return "up";
  return position == "left" ? "right" : "left";
}

}  // namespace

QVariantMap OverlayBridge::dot_center(const QString& position,
  int screen_width,
  int screen_height) const {
  const double margin = 0.1 * qMin(screen_width, screen_height);
  const double x = position.contains("left") ? margin
    : position.contains("right")             ? screen_width - margin
                                             : screen_width / 2.0;
  const double y = position.startsWith("up") ? margin
    : position.startsWith("down")            ? screen_height - margin
                                             : screen_height / 2.0;
  return {{"x", qRound(x)}, {"y", qRound(y)}};
}

QVariantMap OverlayBridge::profile(const QString& size, const QString& position) const {
  if (size == "Great") {
    const QString dir = expansion(position);
    const int dx = dir == "left" ? -1 : dir == "right" ? 1 : 0;
    const int dy = dir == "up" ? -1 : dir == "down" ? 1 : 0;
    return {
      {"width", 2 * GREAT_HALF},
      {"height", 2 * GREAT_HALF},
      {"dotX", GREAT_HALF - dx * (GREAT_HALF - GREAT_EDGE)},
      {"dotY", GREAT_HALF - dy * (GREAT_HALF - GREAT_EDGE)},
      {"expansion", dir},
    };
  }
  if (size == "Basic") {
    return {{"width", 100}, {"height", 60}, {"dotX", 50}, {"dotY", 30}, {"expansion", ""}};
  }
  // 26 px fits the 25 px spinner ring
  return {{"width", 26}, {"height", 26}, {"dotX", 13}, {"dotY", 13}, {"expansion", ""}};
}
