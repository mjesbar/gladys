#include <QCoreApplication>
#include <QSocketNotifier>
#include <X11/Xlib.h>
#include <stdio.h>
#include <QDebug>

#include "lib/ipc_server.h"
#include "lib/process_utils.h"
#include "lib/x11_keygrab.h"

int main(int argc, char *argv[]) {
  QCoreApplication app(argc, argv);

  fprintf(stderr, "Daemon: Starting...\n");

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

    IpcClient client("gladys-ipc-server");
    if (client.sendToggle()) {
      fprintf(stderr, "Daemon: Toggle sent.\n");
    } else {
      fprintf(stderr, "Daemon: No server running, launching gladys...\n");
      processUtils.launchGladys();
    }
  });

  return app.exec();
}