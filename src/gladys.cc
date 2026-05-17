// Gladys - Main entry point for the voice assistant application.
// This single executable manages both the UI and the daemon functionality
// through the gladysd hub module.

#include <QApplication>
#include <QDebug>
#include <QRect>
#include <QScreen>
#include <QThread>
#include <QWidget>

#include "lib/modules/gladysd.h"
#include "lib/modules/ui.h"

int main(int argc, char *argv[]) {
  // Force X11 (xcb) platform - not Wayland (Linux only)
#ifdef __linux__
  qputenv("QT_QPA_PLATFORM", "xcb");
#endif

  QApplication app(argc, argv);
  app.setApplicationName("Gladys");
  app.setQuitOnLastWindowClosed(false);

  // Initialize Gladysd Hub (daemon functionality)
  Gladysd *gladysd = Gladysd::instance();
  if (!gladysd->init()) {
    fprintf(stderr, "Gladys: Failed to initialize gladysd hub.\n");
    return 1;
  }

  // Create UI window
  QWidget dummy;
  UI window(&dummy);

  // Connect Gladysd hub signals to UI (direct signals, no IPC needed)
  QObject::connect(gladysd, &Gladysd::toggleRequested, [&]() {
    qDebug() << "Gladys: Toggling visibility";
    window.toggleVisibility();
  });

  QObject::connect(gladysd, &Gladysd::audioLevelUpdated, &window,
                   &UI::updateAudioLevels);

  // Connect window quit request to gladysd shutdown
  QObject::connect(&window, &UI::quitRequested, [&]() {
    fprintf(stderr, "Gladys: Quit requested, shutting down.\n");
    gladysd->shutdown();
    app.quit();
  });

  // Position window
  QScreen *screen = QApplication::primaryScreen();
  if (screen) {
    window.move(960 - window.width() / 2, 10);
    window.setWindowOpacity(0.0);
  }

  window.show();
  window.removeShadow();
  qDebug() << "Gladys: Window shown";

  return app.exec();
}