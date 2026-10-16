// Toggle Module - Global hotkey (Ctrl+Alt+P) detection.

#include "toggle.hpp"

#include "../log/log.hpp"

#ifdef __linux__
#include <X11/Xlib.h>
#include <X11/keysym.h>
#endif
#ifdef __APPLE__
#include <CoreFoundation/CoreFoundation.h>
#endif

#ifdef _WIN32
static Toggle *g_toggle = nullptr; // hook callback has no user data
#endif

Toggle::Toggle(QObject *parent)
    : QObject(parent), m_poll_timer(new QTimer(this)), m_display(nullptr),
      m_keycode(0), m_modifiers(0) {
  m_poll_timer->setInterval(33); // ~30fps, enough for hotkey detection
  connect(m_poll_timer, &QTimer::timeout, this, &Toggle::process_events);
#ifdef __APPLE__
  m_event_tap = nullptr;
  m_run_loop_source = nullptr;
#endif
#ifdef _WIN32
  m_keyboard_hook = nullptr;
#endif
}

Toggle::~Toggle() {
  qCInfo(lcToggle) << "stopped";
#ifdef __linux__
  if (m_display) {
    XCloseDisplay(static_cast<Display *>(m_display));
  }
#endif
#ifdef __APPLE__
  if (m_event_tap) {
    CFMachPortInvalidate(static_cast<CFMachPortRef>(m_event_tap));
    CFRelease(static_cast<CFMachPortRef>(m_event_tap));
  }
  if (m_run_loop_source) {
    CFRelease(static_cast<CFRunLoopSourceRef>(m_run_loop_source));
  }
#endif
#ifdef _WIN32
  if (m_keyboard_hook) {
    UnhookWindowsHookEx(static_cast<HHOOK>(m_keyboard_hook));
  }
  if (g_toggle == this) {
    g_toggle = nullptr;
  }
#endif
}

void Toggle::start_polling() { m_poll_timer->start(); }

// ---------------------------------------------------------------------------
// Linux (X11)
// ---------------------------------------------------------------------------

#ifdef __linux__

bool Toggle::init() {
  Display *d = XOpenDisplay(nullptr);
  if (!d) {
    qCCritical(lcToggle) << "unable to open X display";
    return false;
  }
  m_display = d;

  m_keycode = XKeysymToKeycode(d, XK_P);
  if (!m_keycode) {
    qCCritical(lcToggle) << "key P not mapped";
    return false;
  }
  m_modifiers = ControlMask | Mod1Mask;

  Window root = DefaultRootWindow(d);
  XSelectInput(d, root, KeyPressMask | KeyReleaseMask);
  // Grab with NumLock (Mod2) and CapsLock (Lock) variants too
  const unsigned int variants[] = {0, Mod2Mask, LockMask, Mod2Mask | LockMask};
  for (unsigned int extra : variants) {
    XGrabKey(d, m_keycode, m_modifiers | extra, root, True, GrabModeAsync,
             GrabModeAsync);
  }
  XFlush(d);
  qCInfo(lcToggle) << "started, hotkey registered";
  return true;
}

void Toggle::process_events() {
  if (!m_display) {
    return;
  }
  Display *d = static_cast<Display *>(m_display);
  const unsigned int relevant = ShiftMask | LockMask | ControlMask | Mod1Mask |
                                Mod2Mask | Mod3Mask | Mod4Mask | Mod5Mask;
  XEvent event;
  while (XPending(d)) {
    XNextEvent(d, &event);
    if (event.type != KeyPress) {
      continue;
    }
    unsigned int mods = event.xkey.state & relevant;
    if (event.xkey.keycode == m_keycode && (mods & m_modifiers) == m_modifiers) {
      qCInfo(lcToggle) << "hotkey pressed";
      emit KEY_PRESSED();
    }
  }
}

#endif // __linux__

// ---------------------------------------------------------------------------
// macOS (CGEventTap)
// ---------------------------------------------------------------------------

#ifdef __APPLE__

bool Toggle::init() {
  m_keycode = 0x23; // kVK_ANSI_P
  m_modifiers = kCGEventFlagMaskCommand | kCGEventFlagMaskAlternate;

  CGEventMask eventMask = CGEventMaskBit(kCGEventKeyDown);
  CFMachPortRef tap = CGEventTapCreate(kCGHIDEventTap, kCGHeadInsertEventTap,
                                       kCGEventTapOptionDefault, eventMask,
                                       event_tap_callback, this);
  if (!tap) {
    qCCritical(lcToggle) << "CGEventTapCreate failed. "
                            "Grant Accessibility permission in System Preferences.";
    return false;
  }
  m_event_tap = tap;

  CFRunLoopSourceRef source =
      CFMachPortCreateRunLoopSource(kCFAllocatorDefault, tap, 0);
  if (!source) {
    qCCritical(lcToggle) << "failed to create run loop source";
    return false;
  }
  m_run_loop_source = source;
  CFRunLoopAddSource(CFRunLoopGetCurrent(), source, kCFRunLoopCommonModes);
  qCInfo(lcToggle) << "started, hotkey registered";
  return true;
}

void Toggle::process_events() {
  // macOS is callback-driven (CGEventTap); no polling needed.
}

CGEventRef Toggle::event_tap_callback(CGEventTapProxy proxy, CGEventType type,
                                      CGEventRef event, void *userInfo) {
  (void)proxy;
  if (type == kCGEventKeyDown) {
    Toggle *self = static_cast<Toggle *>(userInfo);
    CGKeyCode keyCode = static_cast<CGKeyCode>(
        CGEventGetIntegerValueField(event, kCGKeyboardEventKeycode));
    CGEventFlags flags = CGEventGetFlags(event);
    if (keyCode == self->m_keycode &&
        (flags & self->m_modifiers) == self->m_modifiers) {
      qCInfo(lcToggle) << "hotkey pressed";
      emit self->KEY_PRESSED();
    }
  }
  return event; // pass through
}

#endif // __APPLE__

// ---------------------------------------------------------------------------
// Windows (SetWindowsHookEx)
// ---------------------------------------------------------------------------

#ifdef _WIN32

bool Toggle::init() {
  m_keycode = 'P';
  m_modifiers = MOD_CONTROL | MOD_ALT;
  g_toggle = this;

  HHOOK hook = SetWindowsHookEx(WH_KEYBOARD_LL, low_level_keyboard_proc,
                                GetModuleHandle(NULL), 0);
  if (!hook) {
    qCCritical(lcToggle) << "SetWindowsHookEx failed (error" << GetLastError()
                         << "). Run as Administrator.";
    g_toggle = nullptr;
    return false;
  }
  m_keyboard_hook = hook;
  qCInfo(lcToggle) << "started, hotkey registered";
  return true;
}

void Toggle::process_events() {
  MSG msg;
  while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }
}

LRESULT CALLBACK Toggle::low_level_keyboard_proc(int nCode, WPARAM wParam,
                                                 LPARAM lParam) {
  if (nCode == HC_ACTION && (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN)) {
    KBDLLHOOKSTRUCT *p = reinterpret_cast<KBDLLHOOKSTRUCT *>(lParam);
    bool ctrl = GetAsyncKeyState(VK_CONTROL) & 0x8000;
    bool alt = GetAsyncKeyState(VK_MENU) & 0x8000;
    if (p->vkCode == 'P' && ctrl && alt && g_toggle) {
      qCInfo(lcToggle) << "hotkey pressed";
      emit g_toggle->KEY_PRESSED();
    }
  }
  return CallNextHookEx(NULL, nCode, wParam, lParam);
}

#endif // _WIN32
