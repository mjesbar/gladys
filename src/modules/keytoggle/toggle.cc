// Toggle Module - Global hotkey (Ctrl+Alt+P) detection.

#include "toggle.hpp"

#include "../log/log.hpp"

#ifdef __linux__
#include <fcntl.h>
#include <linux/input.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include <cstring>
#include <string>
#endif
#ifdef __APPLE__
#include <CoreFoundation/CoreFoundation.h>
#endif

#ifdef _WIN32
static Toggle* g_toggle = nullptr;  // hook callback has no user data
#endif

Toggle::Toggle(QObject* parent)
    : QObject(parent), m_poll_timer(new QTimer(this)), m_keycode(0), m_modifiers(0) {
  m_poll_timer->setInterval(33);  // ~30fps, enough for hotkey detection
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
  for (int fd : m_fds) {
    close(fd);
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

// macOS is callback-driven: its tap runs on the thread's CFRunLoop, no timer.
void Toggle::start() {
  const bool ok = init();
#ifndef __APPLE__
  if (ok) {
    m_poll_timer->start();
  }
#endif
  emit STARTED(ok);
}

// ---------------------------------------------------------------------------
// Linux (evdev /dev/input)
// ---------------------------------------------------------------------------

#ifdef __linux__

// A device is a keyboard if it reports every key of the hotkey.
static bool is_keyboard(int fd) {
  unsigned char keys[KEY_MAX / 8 + 1];
  std::memset(keys, 0, sizeof keys);
  if (ioctl(fd, EVIOCGBIT(EV_KEY, sizeof keys), keys) < 0) {
    return false;
  }
  for (int code : {KEY_P, KEY_LEFTCTRL, KEY_LEFTALT}) {
    if (!((keys[code / 8] >> (code % 8)) & 1)) {
      return false;
    }
  }
  return true;
}

bool Toggle::init() {
  for (int i = 0; i < 64; ++i) {
    std::string path = "/dev/input/event" + std::to_string(i);
    int fd = open(path.c_str(), O_RDONLY | O_NONBLOCK | O_CLOEXEC);
    if (fd < 0) {
      continue;
    }
    if (is_keyboard(fd)) {
      m_fds.push_back(fd);
    } else {
      close(fd);
    }
  }
  if (m_fds.empty()) {
    qCCritical(lcToggle) << "no readable keyboard in /dev/input "
                            "(add user to the input group)";
    return false;
  }
  m_ctrl_down = false;
  m_alt_down = false;
  qCInfo(lcToggle) << "started, hotkey registered on" << m_fds.size() << "keyboard(s)";
  return true;
}

void Toggle::process_events() {
  input_event ev;
  for (int fd : m_fds) {
    while (read(fd, &ev, sizeof ev) == sizeof ev) {
      if (ev.type != EV_KEY) {
        continue;
      }
      if (ev.code == KEY_LEFTCTRL || ev.code == KEY_RIGHTCTRL) {
        m_ctrl_down = ev.value != 0;
      } else if (ev.code == KEY_LEFTALT || ev.code == KEY_RIGHTALT) {
        m_alt_down = ev.value != 0;
      } else if (ev.code == KEY_P && ev.value == 1 && m_ctrl_down && m_alt_down) {
        qCInfo(lcToggle) << "hotkey pressed";
        emit KEY_PRESSED();
      }
    }
  }
}

#endif  // __linux__

// ---------------------------------------------------------------------------
// macOS (CGEventTap)
// ---------------------------------------------------------------------------

#ifdef __APPLE__

bool Toggle::init() {
  m_keycode = 0x23;  // kVK_ANSI_P
  m_modifiers = kCGEventFlagMaskControl | kCGEventFlagMaskAlternate;

  CGEventMask eventMask = CGEventMaskBit(kCGEventKeyDown);
  CFMachPortRef tap = CGEventTapCreate(kCGHIDEventTap,
    kCGHeadInsertEventTap,
    kCGEventTapOptionDefault,
    eventMask,
    event_tap_callback,
    this);
  if (!tap) {
    qCCritical(lcToggle) << "CGEventTapCreate failed. "
                            "Grant Accessibility permission in System Preferences.";
    return false;
  }
  m_event_tap = tap;

  CFRunLoopSourceRef source = CFMachPortCreateRunLoopSource(kCFAllocatorDefault, tap, 0);
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

CGEventRef Toggle::event_tap_callback(
  CGEventTapProxy proxy, CGEventType type, CGEventRef event, void* userInfo) {
  (void)proxy;
  if (type == kCGEventKeyDown) {
    Toggle* self = static_cast<Toggle*>(userInfo);
    CGKeyCode keyCode =
      static_cast<CGKeyCode>(CGEventGetIntegerValueField(event, kCGKeyboardEventKeycode));
    CGEventFlags flags = CGEventGetFlags(event);
    if (keyCode == self->m_keycode && (flags & self->m_modifiers) == self->m_modifiers) {
      qCInfo(lcToggle) << "hotkey pressed";
      emit self->KEY_PRESSED();
    }
  }
  return event;  // pass through
}

#endif  // __APPLE__

// ---------------------------------------------------------------------------
// Windows (SetWindowsHookEx)
// ---------------------------------------------------------------------------

#ifdef _WIN32

bool Toggle::init() {
  m_keycode = 'P';
  m_modifiers = MOD_CONTROL | MOD_ALT;
  g_toggle = this;

  HHOOK hook = SetWindowsHookEx(WH_KEYBOARD_LL, low_level_keyboard_proc, GetModuleHandle(NULL), 0);
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

LRESULT CALLBACK Toggle::low_level_keyboard_proc(int nCode, WPARAM wParam, LPARAM lParam) {
  if (nCode == HC_ACTION && (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN)) {
    KBDLLHOOKSTRUCT* p = reinterpret_cast<KBDLLHOOKSTRUCT*>(lParam);
    bool ctrl = GetAsyncKeyState(VK_CONTROL) & 0x8000;
    bool alt = GetAsyncKeyState(VK_MENU) & 0x8000;
    if (p->vkCode == 'P' && ctrl && alt && g_toggle) {
      qCInfo(lcToggle) << "hotkey pressed";
      emit g_toggle->KEY_PRESSED();
    }
  }
  return CallNextHookEx(NULL, nCode, wParam, lParam);
}

#endif  // _WIN32
