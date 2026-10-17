// Audio Bars - Turns STT audio levels into the 20 wave bar heights for QML.

#include "audio_bars.hpp"

#include <cmath>

static const int EDGE_MIN_HEIGHT = 2;
static const int EDGE_MAX_HEIGHT = 20;
static const int CENTER_MIN_HEIGHT = 10;
static const int CENTER_MAX_HEIGHT = 48;

AudioBars::AudioBars(QObject *parent)
    : QObject(parent), m_heights(BAR_COUNT, 0.0) {}

QList<qreal> AudioBars::heights() const { return m_heights; }

// Bell curve: tallest bars at the center. Level is eased (quad in) per bar.
void AudioBars::update_levels(const QVector<double> &levels) {
  if (levels.isEmpty()) {
    m_heights.fill(0.0);
  } else {
    for (int i = 0; i < BAR_COUNT; ++i) {
      int level_index = i * levels.size() / BAR_COUNT;
      double level = std::fmin(std::fmax(levels[level_index], 0.0), 1.0);
      double eased = level * level;

      double position = static_cast<double>(i) / (BAR_COUNT - 1);
      double distance = std::fabs(position - 0.5) * 2.0;
      double bell = 1.0 - std::pow(distance, 1.5);

      double min_h = EDGE_MIN_HEIGHT + bell * (CENTER_MIN_HEIGHT - EDGE_MIN_HEIGHT);
      double max_h = EDGE_MAX_HEIGHT + bell * (CENTER_MAX_HEIGHT - EDGE_MAX_HEIGHT);
      m_heights[i] = static_cast<int>(min_h + eased * (max_h - min_h));
    }
  }
  emit HEIGHTS_CHANGED();
}
