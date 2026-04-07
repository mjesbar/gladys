#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

int main() {
    Display* display = XOpenDisplay(NULL);
    if (!display) {
        fprintf(stderr, "Unable to open X display\n");
        return 1;
    }

    Window root = DefaultRootWindow(display);
    KeyCode keycode = XKeysymToKeycode(display, XK_I);
    unsigned int modifiers = ControlMask;

    // Grab Ctrl+I
    XGrabKey(display, keycode, modifiers, root, True, GrabModeAsync, GrabModeAsync);
    // Also grab with NumLock/CapsLock to be robust
    XGrabKey(display, keycode, modifiers | Mod2Mask, root, True, GrabModeAsync, GrabModeAsync);
    XGrabKey(display, keycode, modifiers | LockMask, root, True, GrabModeAsync, GrabModeAsync);
    XGrabKey(display, keycode, modifiers | Mod2Mask | LockMask, root, True, GrabModeAsync, GrabModeAsync);

    XEvent event;
    while (1) {
        XNextEvent(display, &event);
        if (event.type == KeyPress) {
            // Use fork and exec to avoid blocking the daemon
            if (fork() == 0) {
                // Child process
                execlp("sh", "sh", "-c", "QT_QPA_PLATFORM=xcb ./gladys", (char*)NULL);
                _exit(1);
            }
        }
    }

    XCloseDisplay(display);
    return 0;
}
