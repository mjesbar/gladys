#include <QCoreApplication>
#include <QSocketNotifier>
#include <X11/Xlib.h>
#include <stdio.h>
#include <QDebug>

#include "lib/ipc.h"
#include "lib/process.h"
#include "lib/keygrab.h"
#include "lib/stt.h"

int main(int argc, char *argv[]) {
  QCoreApplication app(argc, argv);

  fprintf(stderr, "Daemon: Starting...\n");

  // Load the STT model
  std::string model_path = "./bin/models/sherpa-onnx-streaming-zipformer-es-kroko-2025-08-06";
  if (!SST::load(model_path)) {
      fprintf(stderr, "Daemon: Failed to load STT model.\n");
      return 1;
  }

  X11KeyGrab keyGrab;
  Display *display = XOpenDisplay(NULL);
  if (!display) {
    fprintf(stderr, "Daemon: Unable to open X display\n");
    return 1;
  }

  if (!keyGrab.init(display)) {
    return 1;
  }

  fprintf(stderr, "Daemon: Listening for Ctrl+Alt+P...\n");

  ProcessUtils processUtils;
  processUtils.launchGladys();

  int x11_fd = ConnectionNumber(display);
  QSocketNotifier *notifier =
      new QSocketNotifier(x11_fd, QSocketNotifier::Read, &app);

  QObject::connect(notifier, &QSocketNotifier::activated, [&keyGrab, &processUtils]() {
    keyGrab.processEvents();
  });

  QObject::connect(&keyGrab, &X11KeyGrab::keyPressed, [&]() {
    fprintf(stderr, "Daemon: Ctrl+Alt+P detected!\n");

    static bool stt_running = false;

    if (stt_running) {
        SST::stop();
        stt_running = false;
    } else {
        SST::start();
        stt_running = true;
    }

    IPCClient client("gladys-ipc-server");
    if (client.sendToggle()) {
      fprintf(stderr, "Daemon: Toggle sent.\n");
    } else {
      fprintf(stderr, "Daemon: No server running, launching gladys...\n");
      processUtils.launchGladys();
    }
  });

  return app.exec();
}