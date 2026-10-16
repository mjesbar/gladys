// App Window - Frameless overlay anchored above the bottom edge, tray icon and settings.

#include "app_window.hpp"

#include <QApplication>
#include <QCoreApplication>
#include <QEasingCurve>
#include <QMenu>
#include <QIcon>
#include <QScreen>

#include "../log/log.hpp"

#ifdef __linux__
#include <X11/Xlib.h> // only to remove the shadow
#endif

static const int NAV_BAR_PADDING = 60; // keeps the mic clear of the OS nav bar

AppWindow::AppWindow(QWidget *parent)
    : QWidget(parent), m_mic(new Mic(this)),
      m_opacity_animation(new QPropertyAnimation(this, "windowOpacity")),
      m_is_expanded(false), m_tray_icon(new QSystemTrayIcon(this)),
      m_tray_menu(new QMenu(this)), m_setting_window(new SettingWindow(nullptr)) {
  setObjectName("appWindow");
  setWindowTitle("Gladys");
  setWindowFlags(Qt::ToolTip | Qt::FramelessWindowHint |
                 Qt::WindowStaysOnTopHint | Qt::WindowDoesNotAcceptFocus |
                 Qt::NoDropShadowWindowHint);
  setAttribute(Qt::WA_TranslucentBackground);
  setAttribute(Qt::WA_ShowWithoutActivating);
  setAttribute(Qt::WA_NoSystemBackground);
#ifdef __linux__
  setProperty("_kde_no_shadows", true);
  setProperty("_KDE_NET_WM_SHADOW", false);
#endif
  setWindowOpacity(0.0);

  m_mic->setFixedSize(Mic::SIZE);
  resize(Mic::SIZE);

  m_opacity_animation->setDuration(300);
  m_opacity_animation->setEasingCurve(QEasingCurve::InOutQuad);

  // Tray: left-click opens settings; right-click shows the menu (OS-managed).
  m_tray_menu->addAction(tr("Settings"), this, &AppWindow::open_settings);
  m_tray_menu->addAction(tr("Quit"), qApp, &QApplication::quit);
  m_tray_icon->setContextMenu(m_tray_menu);
  m_tray_icon->setIcon(QIcon(":/icons/icon-app.svg"));
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
  int bottom = geo.bottom() - NAV_BAR_PADDING + 1; // window bottom edge
  move(x, bottom - height());
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

void AppWindow::reveal() {
  m_opacity_animation->setStartValue(0.0);
  m_opacity_animation->setEndValue(1.0);
  m_opacity_animation->start();
}

void AppWindow::toggle_visibility() {
  if (m_mic->is_animating()) {
    return;
  }
  m_is_expanded = !m_is_expanded;
  m_mic->set_expanded(m_is_expanded);
}

void AppWindow::open_settings() {
  m_setting_window->show();
  m_setting_window->raise();
  m_setting_window->activateWindow();
}
