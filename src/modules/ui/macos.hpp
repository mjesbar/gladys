// macOS Window - Overlay behavior for the native Cocoa window. No-op outside macOS.

#ifndef UI_MACOS_WINDOW_HPP
#define UI_MACOS_WINDOW_HPP

#include <QtGlobal>

// Hides the Dock icon and keeps the overlay visible when Gladys is inactive.
void configure_macos_overlay(quintptr window_id);

#endif  // UI_MACOS_WINDOW_HPP
