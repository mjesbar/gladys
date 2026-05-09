#include "keygrab.h"
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <stdio.h>

KeyGrab::KeyGrab(QObject *parent)
    : QObject(parent), m_display(nullptr), m_root(0), m_keycode(0), m_modifiers(0) {
}

KeyGrab::~KeyGrab() {
  if (m_display) {
    XCloseDisplay(static_cast<Display*>(m_display));
  }
}

bool KeyGrab::init(void *display) {
  m_display = display;
  if (!m_display) {
    fprintf(stderr, "KeyGrab: display is null\n");
    return false;
  }

  Display *d = static_cast<Display*>(m_display);
  m_root = DefaultRootWindow(d);
  XSelectInput(d, m_root, KeyPressMask | KeyReleaseMask);
  XFlush(d);

  m_keycode = XKeysymToKeycode(d, XK_P);
  m_modifiers = ControlMask | Mod1Mask;

  fprintf(stderr, "KeyGrab: Grabbing Ctrl+Alt+P (keycode=%d)...\n", m_keycode);

  XGrabKey(d, m_keycode, m_modifiers, m_root, True, GrabModeAsync,
           GrabModeAsync);
  XGrabKey(d, m_keycode, m_modifiers | Mod2Mask, m_root, True, GrabModeAsync,
           GrabModeAsync);
  XGrabKey(d, m_keycode, m_modifiers | LockMask, m_root, True, GrabModeAsync,
           GrabModeAsync);
  XGrabKey(d, m_keycode, m_modifiers | Mod2Mask | LockMask, m_root, True,
           GrabModeAsync, GrabModeAsync);

  XFlush(d);
  return true;
}

void KeyGrab::processEvents() {
  if (!m_display) return;

  Display *d = static_cast<Display*>(m_display);
  XEvent event;
  while (XPending(d)) {
    XNextEvent(d, &event);
    if (event.type == KeyPress) {
      unsigned int eventMods =
          event.xkey.state &
          (ShiftMask | LockMask | ControlMask | Mod1Mask | Mod2Mask |
           Mod3Mask | Mod4Mask | Mod5Mask);
      unsigned int eventKeycode = event.xkey.keycode;
      if (eventKeycode == m_keycode && (eventMods & m_modifiers) == m_modifiers) {
        emit keyPressed();
      }
    }
  }
}