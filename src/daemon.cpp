#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/prctl.h>
#include <unistd.h>

int main() {
  // Set parent death signal to SIGTERM
  prctl(PR_SET_PDEATHSIG, SIGTERM);

  Display *display = XOpenDisplay(NULL);
  if (!display) {
    fprintf(stderr, "Unable to open X display\n");
    return 1;
  }

  Window root = DefaultRootWindow(display);
  KeyCode keycode = XKeysymToKeycode(display, XK_I);
  unsigned int modifiers = ControlMask | ShiftMask;

  // Grab Ctrl+I
  XGrabKey(display, keycode, modifiers, root, True, GrabModeAsync,
           GrabModeAsync);
  XGrabKey(display, keycode, modifiers | Mod2Mask, root, True, GrabModeAsync,
           GrabModeAsync);
  XGrabKey(display, keycode, modifiers | LockMask, root, True, GrabModeAsync,
           GrabModeAsync);
  XGrabKey(display, keycode, modifiers | Mod2Mask | LockMask, root, True,
           GrabModeAsync, GrabModeAsync);

  // Get the absolute path of the current executable to find 'gladys'
  char path[1024];
  ssize_t len = readlink("/proc/self/exe", path, sizeof(path) - 1);
  if (len != -1) {
    path[len] = '\0';
    // Remove 'gladysd' from path to get the directory
    for (int i = len - 1; i >= 0; i--) {
      if (path[i] == '/') {
        path[i + 1] = '\0';
        break;
      }
    }
  } else {
    path[0] = '.';
    path[1] = '/';
    path[2] = '\0';
  }

  char gladys_cmd[2048];
  snprintf(gladys_cmd, sizeof(gladys_cmd), "QT_QPA_PLATFORM=xcb %sgladys",
           path);

  XEvent event;
  while (1) {
    XNextEvent(display, &event);
    if (event.type == KeyPress) {
      if (fork() == 0) {
        execlp("sh", "sh", "-c", gladys_cmd, (char *)NULL);
        _exit(1);
      }
    }
  }

  XCloseDisplay(display);
  return 0;
}
