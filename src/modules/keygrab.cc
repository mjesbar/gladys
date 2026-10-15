// KeyGrab Module - Global hotkey detection.
// Cross-platform: X11 (Linux), CGEventTap (macOS), SetWindowsHookEx (Windows).

#include "keygrab.hpp"
#include <stdio.h>

#ifdef __linux__
#include <X11/Xlib.h>
#include <X11/keysym.h>
#endif
#ifdef __APPLE__
#include <CoreGraphics/CoreGraphics.h>
#include <CoreFoundation/CoreFoundation.h>
#endif
#ifdef _WIN32
#include <windows.h>
#endif

// ---------------------------------------------------------------------------
// Linux (X11) implementation
// ---------------------------------------------------------------------------

#ifdef __linux__

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

void KeyGrab::process_events() {
  if (!m_display) return;
  if (!m_timer.isValid() || m_timer.hasExpired(10)) { // 100fps max
    m_timer.restart();
  } else {
    return;
  }

  Display *d = static_cast<Display*>(m_display);
  if (!XPending(d)) return;

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
        emit key_pressed();
      }
    }
  }
}

#endif // __linux__

// ---------------------------------------------------------------------------
// macOS (CGEventTap) implementation
// ---------------------------------------------------------------------------

#ifdef __APPLE__

KeyGrab::KeyGrab(QObject *parent)
    : QObject(parent), m_display(nullptr), m_root(0), m_keycode(0), m_modifiers(0),
      m_event_tap(nullptr), m_run_loop_source(nullptr) {
}

KeyGrab::~KeyGrab() {
  if (m_event_tap) {
    CFMachPortInvalidate(static_cast<CFMachPortRef>(m_event_tap));
    CFRelease(static_cast<CFMachPortRef>(m_event_tap));
  }
  if (m_run_loop_source) {
    CFRunLoopSourceInvalidate(static_cast<CFRunLoopSourceRef>(m_run_loop_source));
    CFRelease(static_cast<CFRunLoopSourceRef>(m_run_loop_source));
  }
}

bool KeyGrab::init(void *display) {
  (void)display; // unused on macOS
  m_display = nullptr;

  // Ctrl on macOS is Cmd key; Alt is Option key
  m_keycode = 0x23; // kVK_ANSI_P
  m_modifiers = kCGEventFlagMaskCommand | kCGEventFlagMaskAlternate;

  fprintf(stderr, "KeyGrab: Setting up CGEventTap for Cmd+Option+P...\n");

  CGEventMask eventMask = CGEventMaskBit(kCGEventKeyDown);
  CFMachPortRef tap = CGEventTapCreate(kCGHIDEventTap,
                                 kCGHeadInsertEventTap,
                                 kCGEventTapOptionDefault,
                                 eventMask,
                                 event_tap_callback,
                                 this);
  if (!tap) {
    fprintf(stderr, "KeyGrab: CGEventTapCreate failed. "
                    "Grant Accessibility permission in System Preferences.\n");
    return false;
  }
  m_event_tap = tap;

  CFRunLoopSourceRef source = CFMachPortCreateRunLoopSource(kCFAllocatorDefault,
                                                             tap, 0);
  if (!source) {
    fprintf(stderr, "KeyGrab: Failed to create run loop source.\n");
    CFRelease(tap);
    m_event_tap = nullptr;
    return false;
  }
  m_run_loop_source = source;

  CFRunLoopAddSource(CFRunLoopGetCurrent(), source,
                     kCFRunLoopCommonModes);
  fprintf(stderr, "KeyGrab: CGEventTap active.\n");
  return true;
}

void KeyGrab::process_events() {
  // macOS uses callback-driven CGEventTap; no polling needed.
}

CGEventRef KeyGrab::event_tap_callback(CGEventTapProxy proxy,
                                     CGEventType type,
                                     CGEventRef event,
                                     void *userInfo) {
  (void)proxy;
  if (type == kCGEventKeyDown) {
    KeyGrab *self = static_cast<KeyGrab*>(userInfo);
    CGKeyCode keyCode = static_cast<CGKeyCode>(
        CGEventGetIntegerValueField(event, kCGKeyboardEventKeycode));
    CGEventFlags flags = CGEventGetFlags(event);

    // Check for Cmd+Option+P
    CGEventFlags required = kCGEventFlagMaskCommand | kCGEventFlagMaskAlternate;
    if (keyCode == self->m_keycode &&
        (flags & required) == required) {
      fprintf(stderr, "KeyGrab: Cmd+Option+P detected!\n");
      emit self->key_pressed();
    }
  }
  return event; // pass through
}

#endif // __APPLE__

// ---------------------------------------------------------------------------
// Windows (SetWindowsHookEx) implementation
// ---------------------------------------------------------------------------

#ifdef _WIN32

// Global pointer for hook callback access
static KeyGrab *g_keyGrabInstance = nullptr;

KeyGrab::KeyGrab(QObject *parent)
    : QObject(parent), m_display(nullptr), m_root(0), m_keycode(0), m_modifiers(0),
      m_keyboard_hook(nullptr) {
}

KeyGrab::~KeyGrab() {
  if (m_keyboard_hook) {
    UnhookWindowsHookEx(static_cast<HHOOK>(m_keyboard_hook));
    m_keyboard_hook = nullptr;
  }
  if (g_keyGrabInstance == this) {
    g_keyGrabInstance = nullptr;
  }
}

bool KeyGrab::init(void *display) {
  (void)display; // unused on Windows
  m_display = nullptr;
  g_keyGrabInstance = this;

  m_keycode = 'P';
  m_modifiers = MOD_CONTROL | MOD_ALT;

  fprintf(stderr, "KeyGrab: Setting up low-level keyboard hook for Ctrl+Alt+P...\n");

  HHOOK hook = SetWindowsHookEx(WH_KEYBOARD_LL,
                                 low_level_keyboard_proc,
                                 GetModuleHandle(NULL),
                                 0);
  if (!hook) {
    fprintf(stderr, "KeyGrab: SetWindowsHookEx failed (error %lu). "
                    "Run as Administrator.\n", GetLastError());
    g_keyGrabInstance = nullptr;
    return false;
  }
  m_keyboard_hook = hook;

  fprintf(stderr, "KeyGrab: Keyboard hook active.\n");
  return true;
}

void KeyGrab::process_events() {
  // Windows uses callback-driven hook; no polling needed.
  // Process message queue to allow hook dispatch.
  MSG msg;
  while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }
}

LRESULT CALLBACK KeyGrab::low_level_keyboard_proc(int nCode,
                                               WPARAM wParam,
                                               LPARAM lParam) {
  if (nCode == HC_ACTION && (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN)) {
    KBDLLHOOKSTRUCT *p = reinterpret_cast<KBDLLHOOKSTRUCT*>(lParam);
    // Check for Ctrl+Alt+P
    bool ctrl = GetAsyncKeyState(VK_CONTROL) & 0x8000;
    bool alt  = GetAsyncKeyState(VK_MENU) & 0x8000;
    if (p->vkCode == 'P' && ctrl && alt) {
      fprintf(stderr, "KeyGrab: Ctrl+Alt+P detected!\n");
      if (g_keyGrabInstance) {
        emit g_keyGrabInstance->key_pressed();
      }
    }
  }
  return CallNextHookEx(NULL, nCode, wParam, lParam);
}

#endif // _WIN32

unsigned int KeyGrab::keycode() const { return m_keycode; }
unsigned int KeyGrab::modifiers() const { return m_modifiers; }
void *KeyGrab::display() const { return m_display; }
