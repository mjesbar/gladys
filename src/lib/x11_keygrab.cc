#include "x11_keygrab.hpp"
#include <X11/keysym.h>
#include <stdio.h>

X11KeyGrab::X11KeyGrab(QObject *parent)
    : QObject(parent), m_display(nullptr), m_root(0), m_keycode(0), m_modifiers(0) {
}

X11KeyGrab::~X11KeyGrab() {
  if (m_display) {
    XCloseDisplay(m_display);
  }
}

bool X11KeyGrab::init(Display *display) {
  m_display = display;
  if (!m_display) {
    fprintf(stderr, "X11KeyGrab: display is null\n");
    return false;
  }

  m_root = DefaultRootWindow(m_display);
  XSelectInput(m_display, m_root, KeyPressMask | KeyReleaseMask);
  XFlush(m_display);

  m_keycode = XKeysymToKeycode(m_display, XK_P);
  m_modifiers = ControlMask | Mod1Mask;

  fprintf(stderr, "X11KeyGrab: Grabbing Ctrl+Alt+P (keycode=%d)...\n", m_keycode);

  XGrabKey(m_display, m_keycode, m_modifiers, m_root, True, GrabModeAsync,
           GrabModeAsync);
  XGrabKey(m_display, m_keycode, m_modifiers | Mod2Mask, m_root, True, GrabModeAsync,
           GrabModeAsync);
  XGrabKey(m_display, m_keycode, m_modifiers | LockMask, m_root, True, GrabModeAsync,
           GrabModeAsync);
  XGrabKey(m_display, m_keycode, m_modifiers | Mod2Mask | LockMask, m_root, True,
           GrabModeAsync, GrabModeAsync);

  XFlush(m_display);
  return true;
}

void X11KeyGrab::processEvents() {
  if (!m_display) return;

  XEvent event;
  while (XPending(m_display)) {
    XNextEvent(m_display, &event);
    if (event.type == KeyPress) {
      unsigned int eventMods =
          event.xkey.state &
          (ShiftMask | LockMask | ControlMask | Mod1Mask | Mod2Mask |
           Mod3Mask | Mod4Mask | Mod5Mask);
      KeyCode eventKeycode = event.xkey.keycode;
      if (eventKeycode == m_keycode && (eventMods & m_modifiers) == m_modifiers) {
        emit keyPressed();
      }
    }
  }
}