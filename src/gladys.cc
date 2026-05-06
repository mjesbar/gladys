#include <QApplication>
#include <QDebug>
#include <QRect>
#include <QScreen>
#include <QWidget>

#include "lib/window.h"
#include "lib/ipc.h"

int main(int argc, char *argv[]) {
  // Force X11 (xcb) platform - not Wayland
  qputenv("QT_QPA_PLATFORM", "xcb");

  QApplication app(argc, argv);
  app.setApplicationName("Gladys");
  app.setQuitOnLastWindowClosed(false);

  IPCServer server("gladys-ipc-server");
  if (!server.start()) {
    return 1;
  }

  QWidget dummy;
  GladysWindow window(&dummy);

  QObject::connect(&server, &IPCServer::toggleRequested, [&]() {
    qDebug() << "Gladys: Toggling visibility";
    window.toggleVisibility();
  });

  QScreen *screen = QApplication::primaryScreen();
  if (screen) {
    QRect rect = screen->availableGeometry();
    int x = rect.left() + (rect.width() - window.width()) / 2;
    int y = 10;
    window.setGeometry(x, y, window.width(), window.height());
    window.setWindowOpacity(0.0);
  }

  window.show();
  window.removeShadow();
  qDebug() << "Gladys: Window shown";

  return app.exec();
}
