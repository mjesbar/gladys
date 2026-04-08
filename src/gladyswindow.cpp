#include "gladyswindow.h"
#include "qnamespace.h"
#include <QApplication>
#include <QDebug>
#include <QImage>
#include <QKeySequence>
#include <QPainter>
#include <QPalette>
#include <QPoint>
#include <QRegion>
#include <QScreen>

#include <QAction>
#include <QIcon>
#include <QMenu>

GladysWindow::GladysWindow(QWidget *parent)
    : QWidget(parent), m_positionAnimation(new QPropertyAnimation(this, "pos")),
      m_opacityAnimation(new QPropertyAnimation(this, "windowOpacity")),
      m_targetYVisible(30), m_targetYSubtle(10), m_isProminent(false),
      m_originalSize(size()), m_trayIcon(new QSystemTrayIcon(this)),
      m_trayMenu(new QMenu(this)) {
  setFixedSize(60, 60);
  setWindowTitle("Gladys");
  setWindowFlags(Qt::ToolTip | Qt::FramelessWindowHint |
                 Qt::WindowStaysOnTopHint | Qt::WindowDoesNotAcceptFocus |
                 Qt::NoDropShadowWindowHint);
  setAttribute(Qt::WA_TranslucentBackground);
  setAttribute(Qt::WA_ShowWithoutActivating);

  QPalette palette = this->palette();
  palette.setColor(QPalette::Window, Qt::transparent);
  setPalette(palette);

  // Tray Setup
  QAction *quitAction = m_trayMenu->addAction("Quit");
  connect(quitAction, &QAction::triggered, qApp, &QApplication::quit);
  m_trayIcon->setContextMenu(m_trayMenu);
  m_trayIcon->setIcon(QIcon("icons/mic-light.png"));
  connect(m_trayIcon, &QSystemTrayIcon::activated, this,
          &GladysWindow::onTrayIconActivated);
  m_trayIcon->show();

  // Setup position animation
  m_positionAnimation->setDuration(250); // 0.25 seconds
  m_positionAnimation->setEasingCurve(QEasingCurve::OutCubic);

  // Setup opacity animation
  m_opacityAnimation->setDuration(250); // 0.25 seconds
  m_opacityAnimation->setEasingCurve(QEasingCurve::OutCubic);
  connect(m_opacityAnimation, &QPropertyAnimation::finished, this,
          &GladysWindow::handleOpacityAnimationFinished);
}

GladysWindow::~GladysWindow() {
  delete m_positionAnimation;
  delete m_opacityAnimation;
}

void GladysWindow::toggleVisibility() {
  if (m_positionAnimation->state() == QPropertyAnimation::Running ||
      m_opacityAnimation->state() == QPropertyAnimation::Running) {
    return; // Avoid re-triggering while animating
  }

  QPoint startPos = pos();
  QPoint endPos;

  qreal startOpacity;
  qreal endOpacity;

  if (m_isProminent) {
    // Transition to subtle (fading out and moving up slightly)
    endPos = QPoint(startPos.x(), m_targetYSubtle);
    startOpacity = 1.0;
    endOpacity = 0.0; // Fully transparent
  } else {
    // Transition to prominent (fading in and moving down)
    setFixedSize(60, 60); // Use explicit size instead of m_originalSize
    endPos = QPoint(startPos.x(), m_targetYVisible);
    startOpacity = 0.0;
    endOpacity = 1.0; // Fully opaque
  }

  m_positionAnimation->setStartValue(startPos);
  m_positionAnimation->setEndValue(endPos);
  m_positionAnimation->start();

  m_opacityAnimation->setStartValue(startOpacity);
  m_opacityAnimation->setEndValue(endOpacity);
  m_opacityAnimation->start();

  m_isProminent = !m_isProminent;
}

void GladysWindow::handleOpacityAnimationFinished() {
  if (!m_isProminent) {
    // If we just finished fading out to the subtle state, resize to 1x1
    setFixedSize(1, 1);
  }
}

void GladysWindow::onTrayIconActivated(
    QSystemTrayIcon::ActivationReason reason) {
  if (reason == QSystemTrayIcon::Trigger) {
    toggleVisibility();
  }
}

void GladysWindow::paintEvent(QPaintEvent *event) {
  QPainter p(this);
  p.setRenderHint(QPainter::Antialiasing);

  // Force absolute transparency for the background using Source composition
  // mode
  p.setCompositionMode(QPainter::CompositionMode_Source);
  p.fillRect(rect(), Qt::transparent);
  p.setCompositionMode(QPainter::CompositionMode_SourceOver);

  p.setBrush(Qt::white);
  p.setPen(Qt::NoPen);
  p.drawEllipse(0, 0, 60, 60);

  QImage image("icons/mic-light.png");
  if (!image.isNull()) {
    QPixmap pix = QPixmap::fromImage(
        image.scaled(32, 32, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    int x = 14;
    int y = 14;
    p.drawPixmap(x, y, pix);
  }
}
