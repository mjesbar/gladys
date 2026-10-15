#include "ui.hpp"
#include "settings/settings_window.hpp"
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

// Positions are computed dynamically in reposition_on_primary_screen()

UI::UI(QWidget *parent)
    : QWidget(parent), m_position_animation(new QPropertyAnimation(this, "pos")),
      m_opacity_animation(new QPropertyAnimation(this, "windowOpacity")),
      m_size_animation(new QPropertyAnimation(this, "size")),
      m_target_y_visible(30), m_target_y_subtle(10), m_is_prominent(false),
      m_original_size(size()), m_tray_icon(new QSystemTrayIcon(this)),
      m_tray_menu(new QMenu(this)) {
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
  m_settings_window = new SettingsWindow(nullptr);

  // Tray menu: Settings, Quit
  QAction *settingsAction = m_tray_menu->addAction("Settings");
  connect(settingsAction, &QAction::triggered, this, &UI::open_settings);
  m_tray_menu->addSeparator();
  QAction *quitAction = m_tray_menu->addAction("Quit");
  connect(quitAction, &QAction::triggered, this, &UI::quit_app);
  m_tray_icon->setContextMenu(m_tray_menu);
  m_tray_icon->setIcon(QIcon(iconsPath() + "/icon-app.svg"));
  // Left-click opens settings; right-click shows context menu
  connect(m_tray_icon, &QSystemTrayIcon::activated, this,
          [this](QSystemTrayIcon::ActivationReason reason) {
            if (reason == QSystemTrayIcon::Trigger) {
              open_settings();
            }
          });
  m_tray_icon->show();

  m_position_animation->setDuration(300);
  m_position_animation->setEasingCurve(QEasingCurve::InOutQuad);

  m_opacity_animation->setDuration(300);
  m_opacity_animation->setEasingCurve(QEasingCurve::InOutQuad);
  connect(m_opacity_animation, &QPropertyAnimation::finished, this,
          &UI::handle_opacity_animation_finished);

  m_scale_animation = new QVariantAnimation(this);
  m_scale_animation->setDuration(300);
  m_scale_animation->setEasingCurve(QEasingCurve::InOutQuad);
  connect(m_scale_animation, &QVariantAnimation::valueChanged, this,
          [this](const QVariant &) { update(); });
  connect(m_scale_animation, &QVariantAnimation::finished, this, [this]() {
    if (!m_is_prominent) {
      m_audio_levels.clear();
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
    m_mic_pixmap = QPixmap::fromImage(
        image.scaled(48, 48, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    m_scaled_mic_pixmap = m_mic_pixmap; // Cache for full size
  }

  // Compute initial position from primary screen geometry
  reposition_on_primary_screen();
}

void UI::reposition_on_primary_screen() {
  QScreen *primary = QApplication::primaryScreen();
  if (!primary) return;

  QRect geo = primary->availableGeometry();
  int centerX = geo.x() + geo.width() / 2;

  m_prominent_pos = QPoint(centerX - WAVE_TOTAL_WIDTH / 2, geo.y() + 66);
  m_subtle_pos = QPoint(centerX - WAVE_TOTAL_WIDTH / 2, geo.y() + 6);

  // If no animation is running, move immediately
  if (m_position_animation->state() != QPropertyAnimation::Running) {
    move(m_is_prominent ? m_prominent_pos : m_subtle_pos);
  }
}

UI::~UI() {
  delete m_position_animation;
  delete m_opacity_animation;
  delete m_size_animation;
  delete m_settings_window;
}

void UI::remove_shadow() {
#ifdef __linux__
  Display *d = XOpenDisplay(NULL);
  XDeleteProperty(d, (Window)winId(),
                  XInternAtom(d, "_KDE_NET_WM_SHADOW", False));
  XCloseDisplay(d);
#endif
}

void UI::toggle_visibility() {
  if (m_position_animation->state() == QPropertyAnimation::Running ||
      m_opacity_animation->state() == QPropertyAnimation::Running ||
      m_scale_animation->state() == QVariantAnimation::Running) {
    return;
  }

  QPoint startPos = pos();
  QPoint endPos;
  qreal startOpacity;
  qreal endOpacity;
  double startScale;
  double endScale;

  if (m_is_prominent) {
    // Collapsing
    endPos = m_subtle_pos;
    startOpacity = PROMINENT_OPACITY;
    endOpacity = SUBTLE_OPACITY;
    startScale = 1.0;
    endScale = 0.5;
  } else {
    // Prominent
    endPos = m_prominent_pos;
    startOpacity = SUBTLE_OPACITY;
    endOpacity = PROMINENT_OPACITY;
    startScale = 0.5;
    endScale = 1.0;
  }

  m_position_animation->setStartValue(startPos);
  m_position_animation->setEndValue(endPos);
  m_position_animation->start();

  m_opacity_animation->setStartValue(startOpacity);
  m_opacity_animation->setEndValue(endOpacity);
  m_opacity_animation->start();

  m_scale_animation->setStartValue(startScale);
  m_scale_animation->setEndValue(endScale);
  m_scale_animation->start();

  m_is_prominent = !m_is_prominent;
}

void UI::handle_opacity_animation_finished() {
  if (!m_is_prominent) {
  }
}

void UI::open_settings() {
  m_settings_window->show();
  m_settings_window->raise();
  m_settings_window->activateWindow();
}

void UI::quit_app() {
  qDebug() << "UI: Quit requested";
  emit quit_requested();
  QApplication::quit();
}

void UI::paintEvent(QPaintEvent *event) {
  QPainter p(this);
  p.setRenderHint(QPainter::Antialiasing);

  // Fill entire widget with transparent to prevent borders showing
  p.fillRect(rect(), Qt::transparent);

  static const int MIC_SIZE_FULL = 48;

  double scale =
      m_scale_animation ? m_scale_animation->currentValue().toDouble() : 1.0;
  int micSize = static_cast<int>(MIC_SIZE_FULL * scale);

  // Draw wave bars during animation or when fully prominent
  if (!m_audio_levels.isEmpty() &&
      (m_scale_animation->state() == QVariantAnimation::Running ||
       scale >= 1.0)) {
    draw_audio_wave(p, QSize(width(), height()), scale);
  }

  // Use cached pixmap (scaled to current size)
  if (!m_mic_pixmap.isNull()) {
    QPixmap *sourcePix;
    if (!m_scaled_mic_pixmap.isNull() && m_scaled_mic_pixmap.size() == QSize(micSize, micSize)) {
      sourcePix = &m_scaled_mic_pixmap;
    } else {
      sourcePix = new QPixmap(m_mic_pixmap.scaled(micSize, micSize, Qt::KeepAspectRatio, Qt::SmoothTransformation));
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

    if (sourcePix != &m_scaled_mic_pixmap) delete sourcePix;
  }

  p.setCompositionMode(QPainter::CompositionMode_SourceOver);
}

void UI::update_audio_levels(const QVector<double> &levels) {
  m_audio_levels = levels;
  update();
}

void UI::draw_audio_wave(QPainter &p, const QSize &size, double scale) {
  if (m_audio_levels.isEmpty()) {
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
    int levelIndex = i * m_audio_levels.size() / WAVE_BAR_COUNT;
    double level =
        (levelIndex < m_audio_levels.size()) ? m_audio_levels[levelIndex] : 0.0;
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
