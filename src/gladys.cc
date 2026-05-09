#include <QApplication>
#include <QDebug>
#include <QRect>
#include <QScreen>
#include <QWidget>

#include "lib/ipc.h"
#include "lib/pm.h"
#include "lib/ui.h"

int main(int argc, char *argv[]) {
  // Force X11 (xcb) platform - not Wayland
  qputenv("QT_QPA_PLATFORM", "xcb");

  QApplication app(argc, argv);
  app.setApplicationName("Gladys");
  app.setQuitOnLastWindowClosed(false);

  // Initialize ProcessManager as window app role
  ProcessManager *pm = ProcessManager::instance();
  pm->init(ProcessManager::RoleWindowApp);

  // Monitor: if daemon exits, close window app
  QObject::connect(pm, &ProcessManager::daemonExited, [&]() {
    fprintf(stderr, "Gladys: daemon exited, closing.\n");
    app.quit();
  });

  // If ydotoold exits, close window app
  QObject::connect(pm, &ProcessManager::ydotoolExited, [&]() {
    fprintf(stderr, "Gladys: ydotoold exited, closing.\n");
    app.quit();
  });

  IPCServer server("gladys-ipc-server");
  if (!server.start()) {
    return 1;
  }

  QWidget dummy;
  UI window(&dummy);

  QObject::connect(&server, &IPCServer::toggleRequested, [&]() {
    qDebug() << "Gladys: Toggling visibility";
    window.toggleVisibility();
  });

  QObject::connect(&server, &IPCServer::audioLevelUpdated, &window,
                   &UI::updateAudioLevels);

  // When quit requested, use ProcessManager to close both
  QObject::connect(&window, &UI::quitRequested, [&]() {
    fprintf(stderr, "Gladys: Quit requested, closing.\n");
    pm->close();
    app.quit();
  });

  QScreen *screen = QApplication::primaryScreen();
  if (screen) {
    window.move(960 - window.width() / 2, 10);
    window.setWindowOpacity(0.0);
  }

  window.show();
  window.removeShadow();
  qDebug() << "Gladys: Window shown (daemon PID: " << pm->daemonPid() << ")";

  return app.exec();
}