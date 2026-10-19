// Tray - System tray icon with Settings and Quit menu (QSystemTrayIcon).

#ifndef UI_TRAY_HPP
#define UI_TRAY_HPP

#include <QObject>
#include <memory>

class QMenu;
class QSystemTrayIcon;

class Tray : public QObject {
  Q_OBJECT

 public:
  explicit Tray(QObject* parent = nullptr);
  ~Tray() override;

 signals:
  void SETTINGS_REQUESTED();

 private:
  QSystemTrayIcon* m_icon;
  std::unique_ptr<QMenu> m_menu;
};

#endif  // UI_TRAY_HPP
