// Mic Widget - Microphone icon with audio wave bars.

#include "mic.hpp"

#include <QApplication>
#include <QCoreApplication>
#include <QEasingCurve>
#include <QGraphicsBlurEffect>
#include <QImage>
#include <QLabel>
#include <QPainter>
#include <QSvgRenderer>
#include <cmath>

#include "../log/log.hpp"

static const int MIC_SIZE_FULL = 48;
static const int WAVE_BAR_COUNT = 20;
static const int WAVE_BAR_WIDTH = 5;
static const int WAVE_BAR_SPACING = 1;
static const int SHADOW_PAD = 12;

Mic::Mic(QWidget *parent)
    : QWidget(parent), m_scale_animation(new QVariantAnimation(this)),
      m_is_expanded(false), m_shadow_size(0) {
  setAttribute(Qt::WA_TransparentForMouseEvents);
  setAutoFillBackground(false);
  qCInfo(lcUi) << "mic started";

  m_scale_animation->setDuration(300);
  m_scale_animation->setEasingCurve(QEasingCurve::InOutQuad);
  connect(m_scale_animation, &QVariantAnimation::valueChanged, this,
          [this](const QVariant &) { update(); });
  connect(m_scale_animation, &QVariantAnimation::finished, this, [this]() {
    if (!m_is_expanded) {
      m_audio_levels.clear();
    }
    update();
  });

  QSvgRenderer renderer(QCoreApplication::applicationDirPath() +
                        "/icons/icon-app.svg");
  if (renderer.isValid()) {
    QImage image(512, 512, QImage::Format_ARGB32);
    image.fill(Qt::transparent);
    QPainter painter(&image);
    renderer.render(&painter);
    painter.end();
    m_mic_pixmap = QPixmap::fromImage(
        image.scaled(MIC_SIZE_FULL, MIC_SIZE_FULL, Qt::KeepAspectRatio,
                     Qt::SmoothTransformation));
    m_scaled_mic_pixmap = m_mic_pixmap;
  }
}

void Mic::set_expanded(bool expanded) {
  m_is_expanded = expanded;
  m_scale_animation->setStartValue(expanded ? 0.5 : 1.0);
  m_scale_animation->setEndValue(expanded ? 1.0 : 0.5);
  m_scale_animation->start();
}

void Mic::update_audio_levels(const QVector<double> &levels) {
  m_audio_levels = levels;
  update();
}

void Mic::paintEvent(QPaintEvent *event) {
  Q_UNUSED(event);
  QPainter p(this);
  p.setRenderHint(QPainter::Antialiasing);
  p.fillRect(rect(), Qt::transparent);

  double scale = m_scale_animation->currentValue().toDouble();
  if (m_scale_animation->currentValue().isNull()) {
    scale = 1.0;
  }
  int micSize = static_cast<int>(MIC_SIZE_FULL * scale);

  if (!m_audio_levels.isEmpty() &&
      (m_scale_animation->state() == QVariantAnimation::Running ||
       scale >= 1.0)) {
    draw_audio_wave(p, scale);
  }

  if (m_mic_pixmap.isNull() || micSize <= 0) {
    return;
  }

  if (m_scaled_mic_pixmap.size() != QSize(micSize, micSize)) {
    m_scaled_mic_pixmap = m_mic_pixmap.scaled(
        micSize, micSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
  }

  int x = (width() - m_scaled_mic_pixmap.width()) / 2;
  int y = (height() - m_scaled_mic_pixmap.height()) / 2;
  draw_shadow(p, x, y, micSize);
  p.drawPixmap(x, y, m_scaled_mic_pixmap);
  p.setCompositionMode(QPainter::CompositionMode_SourceOver);
}

// Blurred yellow halo behind the icon, rebuilt only when the size changes.
void Mic::draw_shadow(QPainter &p, int x, int y, int micSize) {
  if (m_shadow_size != micSize) {
    QLabel shadowLabel(nullptr);
    shadowLabel.setFixedSize(micSize + SHADOW_PAD * 2, micSize + SHADOW_PAD * 2);
    shadowLabel.setPixmap(m_scaled_mic_pixmap);
    shadowLabel.setAlignment(Qt::AlignCenter);
    shadowLabel.setStyleSheet("background: transparent; border: none; padding: 0;");

    QGraphicsBlurEffect blur;
    blur.setBlurRadius(6);
    shadowLabel.setGraphicsEffect(&blur);

    QImage shadowImg = shadowLabel.grab().toImage();
    for (int i = 0; i < shadowImg.width(); ++i) {
      for (int j = 0; j < shadowImg.height(); ++j) {
        QColor px = shadowImg.pixelColor(i, j);
        if (px.alpha() > 0) {
          shadowImg.setPixelColor(i, j, QColor(255, 217, 74, qMin(px.alpha(), 80)));
        }
      }
    }
    m_shadow_pixmap = QPixmap::fromImage(shadowImg);
    m_shadow_size = micSize;
  }
  p.drawPixmap(x - SHADOW_PAD, y - SHADOW_PAD, m_shadow_pixmap);
}

void Mic::draw_audio_wave(QPainter &p, double scale) {
  const int total = WAVE_BAR_COUNT * WAVE_BAR_WIDTH +
                    (WAVE_BAR_COUNT - 1) * WAVE_BAR_SPACING;
  int centerX = width() / 2;
  int centerY = height() / 2;
  int edgeMinHeight = static_cast<int>(2 * scale);
  int edgeMaxHeight = static_cast<int>(20 * scale);
  int centerMinHeight = static_cast<int>(10 * scale);
  int centerMaxHeight = static_cast<int>(48 * scale);
  int startX = centerX - total / 2;

  p.setOpacity(scale);
  p.setBrush(QColor(255, 217, 74)); // Yellow #FFD94A
  p.setPen(Qt::NoPen);

  for (int i = 0; i < WAVE_BAR_COUNT; ++i) {
    int levelIndex = i * m_audio_levels.size() / WAVE_BAR_COUNT;
    double level = (levelIndex < m_audio_levels.size()) ? m_audio_levels[levelIndex] : 0.0;
    double normalizedLevel = qBound(0.0, level, 1.0);

    // Bell curve: 0.0 at edges, 1.0 at center
    double position = static_cast<double>(i) / (WAVE_BAR_COUNT - 1);
    double distanceFromCenter = std::abs(position - 0.5) * 2.0;
    double bellCurve = 1.0 - std::pow(distanceFromCenter, 1.5);

    int baseMinHeight = static_cast<int>(
        edgeMinHeight + bellCurve * (centerMinHeight - edgeMinHeight));
    int baseMaxHeight = static_cast<int>(
        edgeMaxHeight + bellCurve * (centerMaxHeight - edgeMaxHeight));

    double easedLevel = normalizedLevel * normalizedLevel; // easeIn quad
    int barHeight = static_cast<int>(
        baseMinHeight + easedLevel * (baseMaxHeight - baseMinHeight));

    int x = startX + i * (WAVE_BAR_WIDTH + WAVE_BAR_SPACING);
    int yTop = centerY - barHeight / 2;
    p.drawRoundedRect(x, yTop, WAVE_BAR_WIDTH, barHeight,
                      static_cast<qreal>(WAVE_BAR_WIDTH) / 2.0,
                      static_cast<qreal>(WAVE_BAR_WIDTH) / 2.0);
  }
}
