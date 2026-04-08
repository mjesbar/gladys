#include <QCoreApplication>
#include <QLocalSocket>
#include <QProcess>
#include <QSocketNotifier>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <stdio.h>

int main(int argc, char *argv[]) {
  QCoreApplication app(argc, argv);

  fprintf(stderr, "Daemon: Starting...\n");

  Display *display = XOpenDisplay(NULL);
  if (!display) {
    fprintf(stderr, "Daemon: Unable to open X display\n");
    return 1;
  }

  Window root = DefaultRootWindow(display);

  XSelectInput(display, root, KeyPressMask | KeyReleaseMask);
  XFlush(display);

  KeyCode keycode = XKeysymToKeycode(display, XK_P);
  unsigned int modifiers = ControlMask | Mod1Mask;

  fprintf(stderr, "Daemon: Grabbing Ctrl+Alt+P (keycode=%d)...\n", keycode);

  XGrabKey(display, keycode, modifiers, root, True, GrabModeAsync,
           GrabModeAsync);
  XGrabKey(display, keycode, modifiers | Mod2Mask, root, True, GrabModeAsync,
           GrabModeAsync);
  XGrabKey(display, keycode, modifiers | LockMask, root, True, GrabModeAsync,
           GrabModeAsync);
  XGrabKey(display, keycode, modifiers | Mod2Mask | LockMask, root, True,
           GrabModeAsync, GrabModeAsync);

  XFlush(display);

  fprintf(stderr, "Daemon: Listening for Ctrl+Alt+P...\n");

  int x11_fd = ConnectionNumber(display);
  QSocketNotifier *notifier =
      new QSocketNotifier(x11_fd, QSocketNotifier::Read, &app);
  QObject::connect(
      notifier, &QSocketNotifier::activated, [display, keycode, modifiers]() {
        XEvent event;
        while (XPending(display)) {
          XNextEvent(display, &event);
          if (event.type == KeyPress) {
            unsigned int eventMods =
                event.xkey.state &
                (ShiftMask | LockMask | ControlMask | Mod1Mask | Mod2Mask |
                 Mod3Mask | Mod4Mask | Mod5Mask);
            KeyCode eventKeycode = event.xkey.keycode;
            if (eventKeycode == keycode &&
                (eventMods & modifiers) == modifiers) {
              fprintf(stderr, "Daemon: Ctrl+Alt+P detected!\n");

              QLocalSocket socket;
              socket.connectToServer("gladys-ipc-server");
              if (socket.waitForConnected(200)) {
                fprintf(stderr, "Daemon: Connected, sending toggle...\n");
                socket.write("toggle");
                socket.waitForBytesWritten(1000);
                socket.disconnectFromServer();
                fprintf(stderr, "Daemon: Toggle sent.\n");
              } else {
                fprintf(stderr,
                        "Daemon: No server running, launching gladys...\n");
                QProcess *proc = new QProcess();
                QProcessEnvironment env =
                    QProcessEnvironment::systemEnvironment();
                env.insert("QT_QPA_PLATFORM", "xcb");
                proc->setProcessEnvironment(env);
                proc->setProgram("bin/gladys");
                proc->start();
                fprintf(stderr, "Daemon: gladys launched.\n");
              }
            }
          }
        }
      });

  return app.exec();
}
