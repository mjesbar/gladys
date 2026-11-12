// Toggle Module - Global hotkey for the active profile (Ctrl+Alt+P, Fn or AltGr).

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
#include <ApplicationServices/ApplicationServices.h>
#endif

#ifdef _WIN32
static Toggle* g_toggle = nullptr;  // hook callback has no user data
#endif

Toggle::Toggle(QObject* parent) : QObject(parent), m_poll_timer(new QTimer(this)) {
  m_edge_clock.start();
  m_poll_timer->setInterval(33);  // ~30fps, enough for hotkey detection
  connect(m_poll_timer, &QTimer::timeout, this, &Toggle::process_events);
#ifdef __APPLE__
  m_event_tap = nullptr;
  m_run_loop_source = nullptr;
  m_stop_requested = false;
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

// macOS is callback-driven: this thread runs its own CFRunLoop until stop().
void Toggle::set_profile(const QString& name) {
  HotkeyProfile profile = HotkeyProfile::CtrlAltP;
  if (name == "Fn") {
    profile = HotkeyProfile::Fn;
  } else if (name == "AltGr") {
    profile = HotkeyProfile::AltGr;
  }
  m_profile = profile;
  m_held = false;  // the old trigger's release can never match the new one
}

// Presses closer than this to the last accepted edge are contact bounce or a double trigger
// (AltGr and Fn chatter on some keyboards). Releases are never dropped, so Hold cannot get stuck.
static const qint64 DEBOUNCE_MS = 200;

// Press and release are emitted once each: key repeat and bounced presses are ignored.
void Toggle::on_trigger(bool down) {
  if (down == m_held) {
    return;
  }
  if (down && m_edge_clock.elapsed() < DEBOUNCE_MS) {
    return;
  }
  m_held = down;
  m_edge_clock.restart();
  if (down) {
    qCInfo(lcToggle) << "hotkey pressed";
    emit KEY_PRESSED();
  } else {
    qCInfo(lcToggle) << "hotkey released";
    emit KEY_RELEASED();
  }
}

void Toggle::start() {
  const bool ok = init();
#ifndef __APPLE__
  if (ok) {
    m_poll_timer->start();
  }
#endif
  emit STARTED(ok);
#ifdef __APPLE__
  while (ok && !m_stop_requested) {
    CFRunLoopRunInMode(kCFRunLoopDefaultMode, 0.1, false);  // 0.1s bounds stop latency
  }
#endif
}

#ifdef __APPLE__
void Toggle::stop() { m_stop_requested = true; }
#endif

// ---------------------------------------------------------------------------
// Linux (evdev /dev/input)
// ---------------------------------------------------------------------------

#ifdef __linux__

// evdev code of the trigger key. Fn is often kept by the keyboard firmware and may never arrive.
static unsigned int trigger_key(HotkeyProfile profile) {
  switch (profile) {
    case HotkeyProfile::Fn:
      return KEY_FN;
    case HotkeyProfile::AltGr:
      return KEY_RIGHTALT;
    default:
      return KEY_P;
  }
}

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
  qCInfo(lcToggle) << "started, hotkey registered on" << m_fds.size() << "keyboard(s)";
  return true;
}

void Toggle::process_events() {
  input_event ev;
  for (int fd : m_fds) {
    while (read(fd, &ev, sizeof ev) == sizeof ev) {
      if (ev.type != EV_KEY || ev.value == 2) {  // 2 is key repeat
        continue;
      }
      const bool down = ev.value == 1;
      const HotkeyProfile profile = m_profile;
      if (ev.code == trigger_key(profile)) {
        const bool combo = profile != HotkeyProfile::CtrlAltP || (m_ctrl_down && m_alt_down);
        on_trigger(down && combo);
      } else if (ev.code == KEY_LEFTCTRL || ev.code == KEY_RIGHTCTRL) {
        m_ctrl_down = down;
      } else if (ev.code == KEY_LEFTALT || ev.code == KEY_RIGHTALT) {
        m_alt_down = down;
      }
    }
  }
}

#endif  // __linux__

// ---------------------------------------------------------------------------
// macOS (CGEventTap)
// ---------------------------------------------------------------------------

#ifdef __APPLE__

// Event taps need Accessibility. Prompts the user while it is not granted.
static bool has_accessibility() {
  const void* keys[] = {kAXTrustedCheckOptionPrompt};
  const void* values[] = {kCFBooleanTrue};
  CFDictionaryRef options = CFDictionaryCreate(kCFAllocatorDefault, keys, values, 1,
                                               &kCFTypeDictionaryKeyCallBacks,
                                               &kCFTypeDictionaryValueCallBacks);
  const bool trusted = AXIsProcessTrustedWithOptions(options);
  CFRelease(options);
  return trusted;
}

// Virtual key code of the trigger key. Fn and right Option are modifiers: they arrive as flags changed.
static CGKeyCode trigger_key(HotkeyProfile profile) {
  switch (profile) {
    case HotkeyProfile::Fn:
      return 0x3F;  // kVK_Function
    case HotkeyProfile::AltGr:
      return 0x3D;  // kVK_RightOption
    default:
      return 0x23;  // kVK_ANSI_P
  }
}

bool Toggle::init() {
  if (!has_accessibility()) {
    qCCritical(lcToggle) << "Accessibility not granted. Enable Gladys in System Settings "
                            "> Privacy & Security > Accessibility, then restart.";
    return false;
  }

  CGEventMask eventMask = CGEventMaskBit(kCGEventKeyDown) | CGEventMaskBit(kCGEventKeyUp) |
                          CGEventMaskBit(kCGEventFlagsChanged);
  CFMachPortRef tap = CGEventTapCreate(kCGHIDEventTap,
    kCGHeadInsertEventTap,
    kCGEventTapOptionDefault,
    eventMask,
    event_tap_callback,
    this);
  if (!tap) {
    qCCritical(lcToggle) << "CGEventTapCreate failed. Check Accessibility permission.";
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
  Toggle* self = static_cast<Toggle*>(userInfo);
  // macOS disables a tap after a slow callback or user input; a disabled tap stays dead.
  if (type == kCGEventTapDisabledByTimeout || type == kCGEventTapDisabledByUserInput) {
    qCWarning(lcToggle) << "event tap disabled by macOS, re-enabling";
    CGEventTapEnable(static_cast<CFMachPortRef>(self->m_event_tap), true);
    return event;
  }
  const HotkeyProfile profile = self->m_profile;
  const CGKeyCode keyCode =
    static_cast<CGKeyCode>(CGEventGetIntegerValueField(event, kCGKeyboardEventKeycode));
  if (keyCode != trigger_key(profile)) {
    return event;
  }
  const CGEventFlags flags = CGEventGetFlags(event);
  if (profile == HotkeyProfile::CtrlAltP) {
    const CGEventFlags combo = kCGEventFlagMaskControl | kCGEventFlagMaskAlternate;
    if (type == kCGEventKeyDown) {
      self->on_trigger((flags & combo) == combo);
    } else if (type == kCGEventKeyUp) {
      self->on_trigger(false);
    }
  } else if (type == kCGEventFlagsChanged) {
    const CGEventFlags mask =
      profile == HotkeyProfile::Fn ? kCGEventFlagMaskSecondaryFn : kCGEventFlagMaskAlternate;
    self->on_trigger((flags & mask) != 0);
  }
  return event;  // pass through
}

#endif  // __APPLE__

// ---------------------------------------------------------------------------
// Windows (SetWindowsHookEx)
// ---------------------------------------------------------------------------

#ifdef _WIN32

// Virtual key code of the trigger key. Fn never reaches the Windows hook, so it matches nothing.
static DWORD trigger_key(HotkeyProfile profile) {
  switch (profile) {
    case HotkeyProfile::Fn:
      return 0;
    case HotkeyProfile::AltGr:
      return VK_RMENU;
    default:
      return 'P';
  }
}

bool Toggle::init() {
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
  if (nCode == HC_ACTION && g_toggle) {
    const KBDLLHOOKSTRUCT* p = reinterpret_cast<KBDLLHOOKSTRUCT*>(lParam);
    const bool down = wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN;
    const bool up = wParam == WM_KEYUP || wParam == WM_SYSKEYUP;
    const HotkeyProfile profile = g_toggle->m_profile;
    if ((down || up) && p->vkCode == trigger_key(profile)) {
      const bool combo = profile != HotkeyProfile::CtrlAltP ||
        ((GetAsyncKeyState(VK_CONTROL) & 0x8000) && (GetAsyncKeyState(VK_MENU) & 0x8000));
      g_toggle->on_trigger(down && combo);
    }
  }
  return CallNextHookEx(NULL, nCode, wParam, lParam);
}

#endif  // _WIN32
