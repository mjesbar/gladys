// macOS Window - Overlay behavior for the native Cocoa window.

#include "macos_window.hpp"

#import <AppKit/AppKit.h>

void configure_macos_overlay(quintptr window_id) {
  [NSApp setActivationPolicy:NSApplicationActivationPolicyAccessory];
  NSView* view = (__bridge NSView*)reinterpret_cast<void*>(window_id);
  view.window.hidesOnDeactivate = NO;
}
