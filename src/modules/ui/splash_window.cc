// Splash Window - Centered logo with a blue loading spinner, fades in and out.

#include "splash_window.hpp"

#include <QApplication>
#include <QCoreApplication>
#include <QEasingCurve>
#include <QPainter>
#include <QScreen>
#include <QSvgRenderer>

static const int LOGO_SIZE = 120;
static const int SPINNER_SIZE = 24;
static const int SPINNER_STROKE = 3;
static const int SPINNER_GAP = 16;
static const int SPIN_STEP_DEG = 6;
static const int SPIN_INTERVAL_MS = 16;
static const int FADE_MS = 300;
static const QColor SPINNER_COLOR(0x2F, 0x80, 0xED); // blue

SplashWindow::SplashWindow(QWidget *parent)
    : QWidget(parent), m_opacity_animation(new QPropertyAnimation(this, "windowOpacity")),
      m_spin_timer(new QTimer(this)), m_spin_angle(0) {
  setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Tool |
                 Qt::NoDropShadowWindowHint);
  setAttribute(Qt::WA_TranslucentBackground);
  setAttribute(Qt::WA_ShowWithoutActivating);
  setFixedSize(LOGO_SIZE, LOGO_SIZE + SPINNER_GAP + SPINNER_SIZE);
  setWindowOpacity(0.0);

  QPoint center = QApplication::primaryScreen()->geometry().center();
  move(center - rect().center());

  QSvgRenderer renderer(QCoreApplication::applicationDirPath() +
                        "/icons/icon.svg");
  m_logo = QPixmap(LOGO_SIZE, LOGO_SIZE);
  m_logo.fill(Qt::transparent);
  if (renderer.isValid()) {
    QPainter painter(&m_logo);
    renderer.render(&painter, QRectF(0, 0, LOGO_SIZE, LOGO_SIZE));
  }

  m_opacity_animation->setDuration(FADE_MS);
  m_opacity_animation->setEasingCurve(QEasingCurve::InOutQuad);

  m_spin_timer->setInterval(SPIN_INTERVAL_MS);
  connect(m_spin_timer, &QTimer::timeout, this, [this]() {
    m_spin_angle = (m_spin_angle + SPIN_STEP_DEG) % 360;
    update();
  });
}

void SplashWindow::fade_in() {
  show();
  m_spin_timer->start();
  m_opacity_animation->setStartValue(0.0);
  m_opacity_animation->setEndValue(1.0);
  m_opacity_animation->start();
}

void SplashWindow::fade_out() {
  m_opacity_animation->setStartValue(1.0);
  m_opacity_animation->setEndValue(0.0);
  connect(m_opacity_animation, &QPropertyAnimation::finished, this,
          [this]() {
            m_spin_timer->stop();
            hide();
            emit FADE_OUT_FINISHED();
          },
          Qt::SingleShotConnection);
  m_opacity_animation->start();
}

void SplashWindow::paintEvent(QPaintEvent *event) {
  Q_UNUSED(event);
  QPainter p(this);
  p.setRenderHint(QPainter::Antialiasing);
  p.fillRect(rect(), Qt::transparent);

  p.drawPixmap((width() - LOGO_SIZE) / 2, 0, m_logo);

  // Arc rect is inset by half the stroke so the pen stays inside the widget.
  double half = SPINNER_STROKE / 2.0;
  QRectF arc((width() - SPINNER_SIZE) / 2.0 + half,
             LOGO_SIZE + SPINNER_GAP + half, SPINNER_SIZE - SPINNER_STROKE,
             SPINNER_SIZE - SPINNER_STROKE);
  p.setPen(QPen(SPINNER_COLOR, SPINNER_STROKE, Qt::SolidLine, Qt::RoundCap));
  p.drawArc(arc, m_spin_angle * 16, 270 * 16); // Qt angles: 1/16 degree
}
