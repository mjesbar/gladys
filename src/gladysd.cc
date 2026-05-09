#include <QCoreApplication>
#include <QDebug>
#include <QSocketNotifier>
#include <QThread>
#include <QTimer>
#include <X11/Xlib.h>

#include "lib/ipc.h"
#include "lib/keygrab.h"
#include "lib/keytype.h"
#include "lib/pm.h"
#include "lib/stt.h"

int main(int argc, char *argv[]) {
  QCoreApplication app(argc, argv);

  // Initialize ProcessManager as daemon role
  ProcessManager *pm = ProcessManager::instance();
  pm->init(ProcessManager::RoleDaemon);

  fprintf(stderr, "Daemon: Starting...\n");

  // Load the STT model
  std::string model_path =
      "./bin/models/sherpa-onnx-streaming-zipformer-es-kroko-2025-08-06";
  if (!STT::load(model_path)) {
    fprintf(stderr, "Daemon: Failed to load STT model.\n");
    return 1;
  }

  Display *display = XOpenDisplay(NULL);
  if (!display) {
    fprintf(stderr, "Daemon: Unable to open X display\n");
    return 1;
  }

  // KeyGrab stays in main thread (X11 is not thread-safe)
  KeyGrab keyGrab;
  if (!keyGrab.init(display)) {
    return 1;
  }

  fprintf(stderr, "Daemon: Listening for Ctrl+Alt+P...\n");

  // Poll X11 events in main thread using QSocketNotifier
  int x11_fd = ConnectionNumber(display);
  QSocketNotifier *notifier = new QSocketNotifier(x11_fd, QSocketNotifier::Read, &app);
  QObject::connect(notifier, &QSocketNotifier::activated, [&keyGrab]() {
    keyGrab.processEvents();
  });

  pm->launchWindowApp();
  pm->launchYdotool();

  // === STT runs in its own thread ===
  QThread *sttThread = new QThread();
  STT::instance()->moveToThread(sttThread);
  sttThread->start();

  // If window app exits, close daemon
  QObject::connect(pm, &ProcessManager::windowAppExited, [&]() {
    fprintf(stderr, "Daemon: window app exited, closing.\n");
    app.quit();
  });

  // If ydotoold exits, close all
  QObject::connect(pm, &ProcessManager::ydotoolExited, [&]() {
    fprintf(stderr, "Daemon: ydotoold exited, closing.\n");
    app.quit();
  });

  // Handle key press - runs in main thread but calls thread-safe STT methods
  static bool stt_running = false;
  QObject::connect(&keyGrab, &KeyGrab::keyPressed, [&]() {
    fprintf(stderr, "Daemon: Ctrl+Alt+P detected!\n");

    if (stt_running) {
      STT::stop();
      stt_running = false;
    } else {
      STT::start();
      stt_running = true;
    }

    IPCClient client("gladys-ipc-server");
    if (client.sendToggle()) {
      fprintf(stderr, "Daemon: Toggle sent.\n");
    } else {
      fprintf(stderr, "Daemon: No server running, launching gladys...\n");
      pm->launchWindowApp();
    }
  });

  // Forward STT audio levels to IPC
  QObject::connect(STT::instance(), &STT::audioLevelUpdated, [&]() {
    IPCClient client("gladys-ipc-server");
    client.sendAudioLevels(STT::getAudioLevels());
  });

  // Handle transcription in main thread via queued connection
  QObject::connect(STT::instance(), &STT::textReceived, [](const QString &text) {
    KeyType::instance()->push(text);
  });

  // Cleanup threads on quit
  QObject::connect(&app, &QCoreApplication::aboutToQuit, [&]() {
    fprintf(stderr, "Daemon: Cleaning up threads...\n");
    sttThread->quit();
    sttThread->wait();
    XCloseDisplay(display);
  });

  return app.exec();
}
