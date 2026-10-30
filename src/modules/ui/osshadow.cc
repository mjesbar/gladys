// X11 Shadow - Removes the KDE/X11 drop shadow from a native window.

#include "osshadow.hpp"

#ifdef __linux__
#include <X11/Xlib.h>
#endif

void remove_x11_shadow(quintptr window_id) {
#ifdef __linux__
  Display* d = XOpenDisplay(nullptr);
  if (!d) {
    return;
  }
  XDeleteProperty(d, static_cast<Window>(window_id), XInternAtom(d, "_KDE_NET_WM_SHADOW", False));
  XCloseDisplay(d);
#else
  Q_UNUSED(window_id);
#endif
}
