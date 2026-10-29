// X11 Shadow - Removes the KDE/X11 drop shadow from a native window.

#ifndef UI_X11_SHADOW_HPP
#define UI_X11_SHADOW_HPP

#include <QtGlobal>

// Clears _KDE_NET_WM_SHADOW on the window. No-op outside X11.
void remove_x11_shadow(quintptr window_id);

#endif  // UI_X11_SHADOW_HPP
