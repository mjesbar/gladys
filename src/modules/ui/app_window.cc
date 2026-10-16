// App Window - Frameless overlay with position animation, tray icon and settings.

#include "app_window.hpp"

#include <QAction>
#include <QApplication>
#include <QCoreApplication>
#include <QEasingCurve>
#include <QIcon>
#include <QScreen>

#include "../log/log.hpp"

#ifdef __linux__
#include <X11/Xlib.h> // only to remove the shadow
#endif

AppWindow::AppWindow(QWidget *parent)
    : QWidget(parent), m_mic(new Mic(this)),
      m_position_animation(new QPropertyAnimation(this, "pos")),
      m_opacity_animation(new QPropertyAnimation(this, "windowOpacity")),
      m_is_prominent(false), m_tray_icon(new QSystemTrayIcon(this)),
      m_tray_menu(new QMenu(this)), m_setting_window(new SettingWindow(nullptr)) {
  setWindowTitle("Gladys");
  setWindowFlags(Qt::ToolTip | Qt::FramelessWindowHint |
                 Qt::WindowStaysOnTopHint | Qt::WindowDoesNotAcceptFocus |
                 Qt::NoDropShadowWindowHint);
  setAttribute(Qt::WA_TranslucentBackground);
  setAttribute(Qt::WA_ShowWithoutActivating);
  setAttribute(Qt::WA_NoSystemBackground);
  setStyleSheet("*{border:none;background:transparent;}");
#ifdef __linux__
  setProperty("_kde_no_shadows", true);
  setProperty("_KDE_NET_WM_SHADOW", false);
#endif
  setWindowOpacity(0.0);

  m_mic->setFixedSize(Mic::SIZE);
  resize(Mic::SIZE);

  m_position_animation->setDuration(300);
  m_position_animation->setEasingCurve(QEasingCurve::InOutQuad);
  m_opacity_animation->setDuration(300);
  m_opacity_animation->setEasingCurve(QEasingCurve::InOutQuad);

  // Tray: left-click opens settings; menu has Settings and Quit
  QAction *settingsAction = m_tray_menu->addAction("Settings");
  connect(settingsAction, &QAction::triggered, this, &AppWindow::open_settings);
  m_tray_menu->addSeparator();
  QAction *quitAction = m_tray_menu->addAction("Quit");
  connect(quitAction, &QAction::triggered, qApp, &QApplication::quit);
  m_tray_icon->setContextMenu(m_tray_menu);
  m_tray_icon->setIcon(
      QIcon(QCoreApplication::applicationDirPath() + "/icons/icon-app.svg"));
  connect(m_tray_icon, &QSystemTrayIcon::activated, this,
          [this](QSystemTrayIcon::ActivationReason reason) {
            if (reason == QSystemTrayIcon::Trigger) {
              open_settings();
            }
          });
  m_tray_icon->show();

  reposition_on_primary_screen();
  qCInfo(lcUi) << "started";
}

AppWindow::~AppWindow() {
  delete m_setting_window;
  qCInfo(lcUi) << "stopped";
}

Mic *AppWindow::mic() const { return m_mic; }

void AppWindow::reposition_on_primary_screen() {
  QScreen *primary = QApplication::primaryScreen();
  if (!primary) {
    return;
  }
  QRect geo = primary->availableGeometry();
  int x = geo.x() + geo.width() / 2 - width() / 2;
  m_prominent_pos = QPoint(x, geo.y() + 66);
  m_subtle_pos = QPoint(x, geo.y() + 6);

  if (m_position_animation->state() != QPropertyAnimation::Running) {
    move(m_is_prominent ? m_prominent_pos : m_subtle_pos);
  }
}

void AppWindow::remove_shadow() {
#ifdef __linux__
  Display *d = XOpenDisplay(nullptr);
  if (!d) {
    return;
  }
  XDeleteProperty(d, static_cast<Window>(winId()),
                  XInternAtom(d, "_KDE_NET_WM_SHADOW", False));
  XCloseDisplay(d);
#endif
}

void AppWindow::toggle_visibility() {
  if (m_position_animation->state() == QPropertyAnimation::Running ||
      m_opacity_animation->state() == QPropertyAnimation::Running) {
    return;
  }

  bool expand = !m_is_prominent;

  m_position_animation->setStartValue(pos());
  m_position_animation->setEndValue(expand ? m_prominent_pos : m_subtle_pos);
  m_position_animation->start();

  m_opacity_animation->setStartValue(expand ? 0.0 : 1.0);
  m_opacity_animation->setEndValue(expand ? 1.0 : 0.0);
  m_opacity_animation->start();

  m_mic->set_expanded(expand);
  m_is_prominent = expand;
}

void AppWindow::open_settings() {
  m_setting_window->show();
  m_setting_window->raise();
  m_setting_window->activateWindow();
}
