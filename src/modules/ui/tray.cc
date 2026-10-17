// Tray - System tray icon with Settings and Quit menu (QSystemTrayIcon).

#include "tray.hpp"

#include <QApplication>
#include <QIcon>
#include <QMenu>
#include <QSystemTrayIcon>

Tray::Tray(QObject *parent)
    : QObject(parent), m_icon(new QSystemTrayIcon(this)),
      m_menu(std::make_unique<QMenu>()) {
  m_menu->addAction(tr("Settings"), this,
                    [this]() { emit SETTINGS_REQUESTED(); });
  m_menu->addAction(tr("Quit"), []() { QApplication::quit(); });

  m_icon->setIcon(QIcon(":/icons/icon-app.svg"));
  m_icon->setToolTip("Gladys");
  m_icon->setContextMenu(m_menu.get());

  QObject::connect(m_icon, &QSystemTrayIcon::activated, this,
                   [this](QSystemTrayIcon::ActivationReason reason) {
                     if (reason == QSystemTrayIcon::Trigger) {
                       emit SETTINGS_REQUESTED();
                     }
                   });

  m_icon->show();
}

Tray::~Tray() = default;
