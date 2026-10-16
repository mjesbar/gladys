// App Window - Frameless overlay anchored above the bottom edge, tray icon and settings.

#ifndef UI_APP_WINDOW_HPP
#define UI_APP_WINDOW_HPP

#include <QMenu>
#include <QPropertyAnimation>
#include <QSystemTrayIcon>
#include <QWidget>

#include "mic.hpp"
#include "setting_window.hpp"

class AppWindow : public QWidget {
  Q_OBJECT

public:
  explicit AppWindow(QWidget *parent = nullptr);
  ~AppWindow() override;

  Mic *mic() const;
  void reposition_on_primary_screen();
  void remove_shadow();
  void reveal(); // fades the window in

public slots:
  void toggle_visibility();
  void open_settings();

private:
  Mic *m_mic;
  QPropertyAnimation *m_opacity_animation;
  bool m_is_expanded;
  QSystemTrayIcon *m_tray_icon;
  QMenu *m_tray_menu;
  SettingWindow *m_setting_window;
};

#endif // UI_APP_WINDOW_HPP
