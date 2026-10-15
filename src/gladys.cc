// Gladys - Main entry point for the voice assistant application.
// This single executable manages both the UI and the daemon functionality
// through the gladysd hub module.

#include <QApplication>
#include <QDebug>
#include <QThread>
#include <QWidget>

#include "modules/gladysd.hpp"
#include "modules/ui.hpp"

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

  // Create UI window (no parent — top-level window)
  UI window(nullptr);

  // Connect Gladysd hub signals to UI (direct signals, no IPC needed)
  QObject::connect(gladysd, &Gladysd::toggle_requested, [&]() {
    qDebug() << "Gladys: Toggling visibility";
    window.toggle_visibility();
  });

  QObject::connect(gladysd, &Gladysd::audio_level_updated, &window,
                   &UI::update_audio_levels);

  // Connect window quit request to gladysd shutdown
  QObject::connect(&window, &UI::quit_requested, [&]() {
    fprintf(stderr, "Gladys: Quit requested, shutting down.\n");
    gladysd->shutdown();
    app.quit();
  });

  // Position window on primary screen
  window.reposition_on_primary_screen();
  window.setWindowOpacity(0.0);

  window.show();
  window.remove_shadow();
  qDebug() << "Gladys: Window shown";

  return app.exec();
}
