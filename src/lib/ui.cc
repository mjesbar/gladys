#include "ui.h"
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
#include <X11/Xlib.h> // just to remove the shadow

#include "ipc.h"

static const QSize PROMINENT_SIZE = QSize(48, 48);
static const QSize SUBTLE_SIZE = QSize(24, 24);

static const qreal PROMINENT_OPACITY = 1.0;
static const qreal SUBTLE_OPACITY = 0.0;

static const QPoint PROMINENT_POS = QPoint(936, 72);
static const QPoint SUBTLE_POS = QPoint(948, 12);

UI::UI(QWidget *parent)
    : QWidget(parent), m_positionAnimation(new QPropertyAnimation(this, "pos")),
      m_opacityAnimation(new QPropertyAnimation(this, "windowOpacity")),
      m_sizeAnimation(new QPropertyAnimation(this, "size")),
      m_targetYVisible(30), m_targetYSubtle(10), m_isProminent(false),
      m_originalSize(size()), m_trayIcon(new QSystemTrayIcon(this)),
      m_trayMenu(new QMenu(this)) {
  resize(PROMINENT_SIZE);
  setWindowTitle("Gladys");
  setWindowFlags(Qt::ToolTip | Qt::FramelessWindowHint |
                 Qt::WindowStaysOnTopHint | Qt::WindowDoesNotAcceptFocus |
                 Qt::NoDropShadowWindowHint);
  setAttribute(Qt::WA_TranslucentBackground);
  setAttribute(Qt::WA_ShowWithoutActivating);
  // Hint for some X11 window managers
  setProperty("_kde_no_shadows", true);
  setProperty("_KDE_NET_WM_SHADOW", false);

  QPalette palette = this->palette();
  palette.setColor(QPalette::Window, Qt::transparent);
  setPalette(palette);

  QAction *quitAction = m_trayMenu->addAction("Quit");
  connect(quitAction, &QAction::triggered, this, &UI::quitApp);
  m_trayIcon->setContextMenu(m_trayMenu);
  m_trayIcon->setIcon(QIcon("icons/mic-light.png"));
  connect(m_trayIcon, &QSystemTrayIcon::activated, this,
          &UI::onTrayIconActivated);
  m_trayIcon->show();

  m_positionAnimation->setDuration(500);
  m_positionAnimation->setEasingCurve(QEasingCurve::InOutBack);

  m_opacityAnimation->setDuration(500);
  m_opacityAnimation->setEasingCurve(QEasingCurve::InOutBack);
  connect(m_opacityAnimation, &QPropertyAnimation::finished, this,
          &UI::handleOpacityAnimationFinished);

  m_sizeAnimation->setDuration(500);
  m_sizeAnimation->setEasingCurve(QEasingCurve::InOutBack);
}

UI::~UI() {
  delete m_positionAnimation;
  delete m_opacityAnimation;
  delete m_sizeAnimation;
}

void UI::removeShadow() {
  Display *d = XOpenDisplay(NULL);
  XDeleteProperty(d, (Window)winId(),
                  XInternAtom(d, "_KDE_NET_WM_SHADOW", False));
  XCloseDisplay(d);
}

void UI::toggleVisibility() {
  if (m_positionAnimation->state() == QPropertyAnimation::Running ||
      m_opacityAnimation->state() == QPropertyAnimation::Running ||
      m_sizeAnimation->state() == QPropertyAnimation::Running) {
    return;
  }

  QPoint startPos = pos();
  QPoint endPos;

  QSize startSize = size();
  QSize endSize;

  qreal startOpacity;
  qreal endOpacity;

  if (m_isProminent) {
    endPos = SUBTLE_POS;
    endSize = SUBTLE_SIZE;
    startOpacity = PROMINENT_OPACITY;
    endOpacity = SUBTLE_OPACITY;
  } else {
    endPos = PROMINENT_POS;
    endSize = PROMINENT_SIZE;
    startOpacity = SUBTLE_OPACITY;
    endOpacity = PROMINENT_OPACITY;
  }

  m_positionAnimation->setStartValue(startPos);
  m_positionAnimation->setEndValue(endPos);
  m_positionAnimation->start();

  m_sizeAnimation->setStartValue(startSize);
  m_sizeAnimation->setEndValue(endSize);
  m_sizeAnimation->start();

  m_opacityAnimation->setStartValue(startOpacity);
  m_opacityAnimation->setEndValue(endOpacity);
  m_opacityAnimation->start();

  m_isProminent = !m_isProminent;
}

void UI::handleOpacityAnimationFinished() {
  if (!m_isProminent) {
  }
}

void UI::onTrayIconActivated(
    QSystemTrayIcon::ActivationReason reason) {
  if (reason == QSystemTrayIcon::Trigger) {
    toggleVisibility();
  }
}

void UI::quitApp() {
  qDebug() << "UI: Quit requested";
  emit quitRequested();
  QApplication::quit();
}

void UI::paintEvent(QPaintEvent *event) {
  QPainter p(this);
  p.setRenderHint(QPainter::Antialiasing);

  p.setCompositionMode(QPainter::CompositionMode_Source);
  p.fillRect(rect(), Qt::transparent);
  p.setCompositionMode(QPainter::CompositionMode_SourceOver);

  p.setBrush(Qt::white);
  p.setPen(Qt::NoPen);

  int currentCircleSize = width();
  int offset = (width() - currentCircleSize) / 2;
  p.drawEllipse(offset, offset, currentCircleSize, currentCircleSize);

  QImage image("icons/mic-light.png");
  if (!image.isNull()) {
    QPixmap pix = QPixmap::fromImage(
        image.scaled(currentCircleSize * 32 / 48, currentCircleSize * 32 / 48,
                     Qt::KeepAspectRatio, Qt::SmoothTransformation));
    int x = (width() - pix.width()) / 2;
    int y = (height() - pix.height()) / 2;
    p.drawPixmap(x, y, pix);
  }
}