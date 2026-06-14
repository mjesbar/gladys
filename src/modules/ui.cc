#include "ui.h"
#include "settings/settings_window.h"
#include "qnamespace.h"
#include <QApplication>
#include <QDebug>
#include <QDir>
#include <QImage>
#include <QKeySequence>
#include <QPainter>
#include <QPalette>
#include <QPoint>
#include <QRegion>
#include <QScreen>
#include <QSvgRenderer>
#include <QTimer>
#include <cmath>

#include <QAction>
#include <QIcon>
#include <QMenu>
#include <QGraphicsBlurEffect>
#include <QGraphicsOpacityEffect>
#include <QLabel>

#ifdef __linux__
#include <X11/Xlib.h> // just to remove the shadow
#endif

static const int WAVE_BAR_COUNT = 20;
static const int WAVE_BAR_WIDTH = 5;
static const int WAVE_BAR_SPACING = 1;
static const int WAVE_TOTAL_WIDTH =
    WAVE_BAR_COUNT * WAVE_BAR_WIDTH + (WAVE_BAR_COUNT - 1) * WAVE_BAR_SPACING;

static const QSize PROMINENT_SIZE = QSize(WAVE_TOTAL_WIDTH, 60);
static const QSize SUBTLE_SIZE = QSize(WAVE_TOTAL_WIDTH, 60);

static const qreal PROMINENT_OPACITY = 1.0;
static const qreal SUBTLE_OPACITY = 0.0;

static const QPoint PROMINENT_POS = QPoint(960 - WAVE_TOTAL_WIDTH / 2, 66);
static const QPoint SUBTLE_POS = QPoint(960 - WAVE_TOTAL_WIDTH / 2, 6);

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
  setAttribute(Qt::WA_NoSystemBackground);
  setAttribute(Qt::WA_OpaquePaintEvent, false);
  setAutoFillBackground(false);
  setStyleSheet("*{border:none;background:transparent;}");
  // Hint for some X11 window managers
#ifdef __linux__
  setProperty("_kde_no_shadows", true);
  setProperty("_KDE_NET_WM_SHADOW", false);
#endif

  auto iconsPath = []() { return QCoreApplication::applicationDirPath() + "/icons"; };

  QPalette palette = this->palette();
  palette.setColor(QPalette::Window, Qt::transparent);
  setPalette(palette);

  // Settings window (not parented to UI widget; standalone dialog)
  m_settingsWindow = new SettingsWindow(nullptr);

  // Tray menu: Settings, Quit
  QAction *settingsAction = m_trayMenu->addAction("Settings");
  connect(settingsAction, &QAction::triggered, this, &UI::openSettings);
  m_trayMenu->addSeparator();
  QAction *quitAction = m_trayMenu->addAction("Quit");
  connect(quitAction, &QAction::triggered, this, &UI::quitApp);
  m_trayIcon->setContextMenu(m_trayMenu);
  m_trayIcon->setIcon(QIcon(iconsPath() + "/icon-app.svg"));
  // Left-click opens settings; right-click shows context menu
  connect(m_trayIcon, &QSystemTrayIcon::activated, this,
          [this](QSystemTrayIcon::ActivationReason reason) {
            if (reason == QSystemTrayIcon::Trigger) {
              openSettings();
            }
          });
  m_trayIcon->show();

  m_positionAnimation->setDuration(300);
  m_positionAnimation->setEasingCurve(QEasingCurve::InOutQuad);

  m_opacityAnimation->setDuration(300);
  m_opacityAnimation->setEasingCurve(QEasingCurve::InOutQuad);
  connect(m_opacityAnimation, &QPropertyAnimation::finished, this,
          &UI::handleOpacityAnimationFinished);

  m_scaleAnimation = new QVariantAnimation(this);
  m_scaleAnimation->setDuration(300);
  m_scaleAnimation->setEasingCurve(QEasingCurve::InOutQuad);
  connect(m_scaleAnimation, &QVariantAnimation::valueChanged, this,
          [this](const QVariant &) { update(); });
  connect(m_scaleAnimation, &QVariantAnimation::finished, this, [this]() {
    if (!m_isProminent) {
      m_audioLevels.clear();
    }
    update();
  });

  // Pre-load and cache SVG microphone icon to avoid disk I/O on every paint
  QSvgRenderer renderer(iconsPath() + "/icon-app.svg");
  if (renderer.isValid()) {
    QImage image(512, 512, QImage::Format_ARGB32);
    image.fill(Qt::transparent);
    QPainter painter(&image);
    renderer.render(&painter);
    m_micPixmap = QPixmap::fromImage(
        image.scaled(48, 48, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    m_scaledMicPixmap = m_micPixmap; // Cache for full size
  }
}

UI::~UI() {
  delete m_positionAnimation;
  delete m_opacityAnimation;
  delete m_sizeAnimation;
  delete m_settingsWindow;
}

void UI::removeShadow() {
#ifdef __linux__
  Display *d = XOpenDisplay(NULL);
  XDeleteProperty(d, (Window)winId(),
                  XInternAtom(d, "_KDE_NET_WM_SHADOW", False));
  XCloseDisplay(d);
#endif
}

void UI::toggleVisibility() {
  if (m_positionAnimation->state() == QPropertyAnimation::Running ||
      m_opacityAnimation->state() == QPropertyAnimation::Running ||
      m_scaleAnimation->state() == QVariantAnimation::Running) {
    return;
  }

  QPoint startPos = pos();
  QPoint endPos;
  qreal startOpacity;
  qreal endOpacity;
  double startScale;
  double endScale;

  if (m_isProminent) {
    // Collapsing
    endPos = SUBTLE_POS;
    startOpacity = PROMINENT_OPACITY;
    endOpacity = SUBTLE_OPACITY;
    startScale = 1.0;
    endScale = 0.5;
  } else {
    // Prominent
    endPos = PROMINENT_POS;
    startOpacity = SUBTLE_OPACITY;
    endOpacity = PROMINENT_OPACITY;
    startScale = 0.5;
    endScale = 1.0;
  }

  m_positionAnimation->setStartValue(startPos);
  m_positionAnimation->setEndValue(endPos);
  m_positionAnimation->start();

  m_opacityAnimation->setStartValue(startOpacity);
  m_opacityAnimation->setEndValue(endOpacity);
  m_opacityAnimation->start();

  m_scaleAnimation->setStartValue(startScale);
  m_scaleAnimation->setEndValue(endScale);
  m_scaleAnimation->start();

  m_isProminent = !m_isProminent;
}

void UI::handleOpacityAnimationFinished() {
  if (!m_isProminent) {
  }
}

void UI::openSettings() {
  m_settingsWindow->show();
  m_settingsWindow->raise();
  m_settingsWindow->activateWindow();
}

void UI::quitApp() {
  qDebug() << "UI: Quit requested";
  emit quitRequested();
  QApplication::quit();
}

void UI::paintEvent(QPaintEvent *event) {
  QPainter p(this);
  p.setRenderHint(QPainter::Antialiasing);

  // Fill entire widget with transparent to prevent borders showing
  p.fillRect(rect(), Qt::transparent);

  static const int MIC_SIZE_FULL = 48;

  double scale =
      m_scaleAnimation ? m_scaleAnimation->currentValue().toDouble() : 1.0;
  int micSize = static_cast<int>(MIC_SIZE_FULL * scale);

  // Draw wave bars during animation or when fully prominent
  if (!m_audioLevels.isEmpty() &&
      (m_scaleAnimation->state() == QVariantAnimation::Running ||
       scale >= 1.0)) {
    drawAudioWave(p, QSize(width(), height()), scale);
  }

  // Use cached pixmap (scaled to current size)
  if (!m_micPixmap.isNull()) {
    QPixmap *sourcePix;
    if (!m_scaledMicPixmap.isNull() && m_scaledMicPixmap.size() == QSize(micSize, micSize)) {
      sourcePix = &m_scaledMicPixmap;
    } else {
      sourcePix = new QPixmap(m_micPixmap.scaled(micSize, micSize, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }

    int x = (width() - sourcePix->width()) / 2;
    int y = (height() - sourcePix->height()) / 2;

    // Create blurred yellow shadow
    static QPixmap cachedShadow;
    static int cachedSize = 0;
    if (cachedShadow.isNull() || cachedSize != micSize) {
      int shadowPad = 12;
      int labelW = sourcePix->width() + shadowPad * 2;
      int labelH = sourcePix->height() + shadowPad * 2;

      // Create a transparent bg label to hold the blurred pixmap
      QLabel shadowLabel(nullptr);
      shadowLabel.setFixedSize(labelW, labelH);
      shadowLabel.setPixmap(*sourcePix);
      shadowLabel.setAlignment(Qt::AlignCenter);
      shadowLabel.setStyleSheet("background: transparent; border: none; padding: 0;");

      QGraphicsBlurEffect blur;
      blur.setBlurRadius(6);
      shadowLabel.setGraphicsEffect(&blur);

      // Grab the blurred shadow with extra margin
      QPixmap shadow = shadowLabel.grab();
      // Tint to yellow
      QImage shadowImg = shadow.toImage();
      for (int i = 0; i < shadowImg.width(); ++i) {
        for (int j = 0; j < shadowImg.height(); ++j) {
          QColor px = shadowImg.pixelColor(i, j);
          if (px.alpha() > 0) {
            shadowImg.setPixelColor(i, j, QColor(255, 217, 74, qMin(px.alpha(), 80)));
          }
        }
      }
      cachedShadow = QPixmap::fromImage(shadowImg);
      cachedSize = micSize;
    }

    // Draw shadow centered behind icon (x-12, y-12 offsets to center the 24px wider shadow)
    p.drawPixmap(x - 12, y - 12, cachedShadow);
    p.drawPixmap(x, y, *sourcePix);

    if (sourcePix != &m_scaledMicPixmap) delete sourcePix;
  }

  p.setCompositionMode(QPainter::CompositionMode_SourceOver);
}

void UI::updateAudioLevels(const QVector<double> &levels) {
  m_audioLevels = levels;
  update();
}

void UI::drawAudioWave(QPainter &p, const QSize &size, double scale) {
  if (m_audioLevels.isEmpty()) {
    return;
  }

  int centerX = size.width() / 2;
  int centerY = size.height() / 2;
  int maxBarHeight = static_cast<int>(48 * scale);
  int minBarHeight = static_cast<int>(5 * scale);
  int edgeMinHeight = static_cast<int>(2 * scale);
  int edgeMaxHeight = static_cast<int>(20 * scale);
  int centerMinHeight = static_cast<int>(10 * scale);
  int centerMaxHeight = static_cast<int>(48 * scale);

  int startX = centerX - WAVE_TOTAL_WIDTH / 2;

  p.setOpacity(scale);
  p.setBrush(QColor(255, 217, 74)); // Yellow #FFD94A
  p.setPen(Qt::NoPen);

  for (int i = 0; i < WAVE_BAR_COUNT; ++i) {
    // Map each bar to a fixed position in the buffer (waveform pattern)
    int levelIndex = i * m_audioLevels.size() / WAVE_BAR_COUNT;
    double level =
        (levelIndex < m_audioLevels.size()) ? m_audioLevels[levelIndex] : 0.0;
    double normalizedLevel = qBound(0.0, level, 1.0);

    // Bell curve: 0.0 at edges, 1.0 at center
    double position = static_cast<double>(i) / (WAVE_BAR_COUNT - 1);
    double distanceFromCenter = std::abs(position - 0.5) * 2.0;
    double bellCurve = 1.0 - std::pow(distanceFromCenter, 1.5);

    // Base height: edge 2-20px, center 10-48px (bell curve with audio
    // interpolation)
    int baseMinHeight = static_cast<int>(
        edgeMinHeight + bellCurve * (centerMinHeight - edgeMinHeight));
    int baseMaxHeight = static_cast<int>(
        edgeMaxHeight + bellCurve * (centerMaxHeight - edgeMaxHeight));

    // Use individual level for each bar + bell curve modulation
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
