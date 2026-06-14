// KeyType Module - Keyboard typing via clipboard paste.
// Cross-platform: uinput (Linux), CGEvent (macOS), SendInput (Windows).

#include "keytype.h"

#include <QApplication>
#include <QClipboard>
#include <QProcess>

#ifdef __linux__
#include <cstring>
#include <cerrno>
#endif

// Cross-platform sleep (milliseconds)
#ifdef _WIN32
#include <windows.h>
#define SleepMs(ms) Sleep(ms)
#else
#include <unistd.h>
#define SleepMs(ms) usleep((ms) * 1000)
#endif

// Linux KEY_* constants needed on all platforms for shared typing logic.
// On Linux these come from <linux/uinput.h>; define fallbacks elsewhere.
#ifndef KEY_LEFTCTRL
#define KEY_LEFTCTRL 29
#endif
#ifndef KEY_LEFTSHIFT
#define KEY_LEFTSHIFT 42
#endif
#ifndef KEY_A
#define KEY_A 30
#endif
#ifndef KEY_V
#define KEY_V 47
#endif

KeyType *KeyType::s_instance = nullptr;

KeyType::KeyType(QObject *parent)
    : QObject(parent)
#ifdef __linux__
      , m_fd(-1)
#else
      , m_dummy(0)
#endif
{
  initKeyboard();
}

KeyType::~KeyType() {
  closeKeyboard();
}

// ---------------------------------------------------------------------------
// Platform-specific keyboard initialization
// ---------------------------------------------------------------------------

#ifdef __linux__
bool KeyType::initKeyboard() {
  m_fd = open("/dev/uinput", O_WRONLY);
  if (m_fd < 0) {
    m_fd = open("/dev/input/uinput", O_WRONLY);
    if (m_fd < 0) {
      fprintf(stderr, "KeyType: Failed to open uinput: %s\n", strerror(errno));
      return false;
    }
  }

  ioctl(m_fd, UI_SET_EVBIT, EV_KEY);
  ioctl(m_fd, UI_SET_EVBIT, EV_SYN);

  for (int i = 0; i < KEY_CNT; ++i) {
    ioctl(m_fd, UI_SET_KEYBIT, i);
  }

  struct uinput_setup usetup = {};
  usetup.id.bustype = BUS_USB;
  usetup.id.vendor = 0x1234;
  usetup.id.product = 0x5678;
  strncpy(usetup.name, "Gladys Virtual Keyboard", UINPUT_MAX_NAME_SIZE - 1);
  ioctl(m_fd, UI_DEV_SETUP, &usetup);

  if (ioctl(m_fd, UI_DEV_CREATE) < 0) {
    fprintf(stderr, "KeyType: Failed to create uinput: %s\n", strerror(errno));
    close(m_fd);
    m_fd = -1;
    return false;
  }

  fprintf(stderr, "KeyType: uinput initialized\n");
  return true;
}

void KeyType::closeKeyboard() {
  if (m_fd >= 0) {
    ioctl(m_fd, UI_DEV_DESTROY);
    close(m_fd);
    m_fd = -1;
  }
}

void KeyType::sendKey(unsigned int code, bool press) {
  if (m_fd < 0) return;

  struct input_event ev = {};
  ev.type = EV_KEY;
  ev.code = code;
  ev.value = press ? 1 : 0;
  write(m_fd, &ev, sizeof(ev));

  ev.type = EV_SYN;
  ev.code = SYN_REPORT;
  ev.value = 0;
  write(m_fd, &ev, sizeof(ev));
}
#endif // __linux__

#ifdef __APPLE__
// macOS key code mapping from Linux KEY_* constants to CGKeyCode
static CGKeyCode macKeyCode(unsigned int linuxCode) {
  // Map only the keys used by Gladys
  switch (linuxCode) {
    case KEY_LEFTCTRL:  return 0x3B; // kVK_Control
    case KEY_LEFTSHIFT: return 0x38; // kVK_Shift
    case KEY_A:         return 0x00; // kVK_ANSI_A
    case KEY_V:         return 0x09; // kVK_ANSI_V
    default:            return 0x00;
  }
}

static CGEventFlags macModifiers(unsigned int linuxCode) {
  switch (linuxCode) {
    case KEY_LEFTCTRL:  return kCGEventFlagMaskCommand; // Cmd on macOS
    case KEY_LEFTSHIFT: return kCGEventFlagMaskShift;
    default:            return 0;
  }
}

bool KeyType::initKeyboard() {
  fprintf(stderr, "KeyType: Using CGEvent keyboard backend\n");
  return true;
}

void KeyType::closeKeyboard() {
  // Nothing to clean up for CGEvent
}

void KeyType::sendKey(unsigned int code, bool press) {
  CGKeyCode keyCode = macKeyCode(code);
  CGEventRef event = CGEventCreateKeyboardEvent(NULL, keyCode, press);
  if (!event) return;
  CGEventPost(kCGHIDEventTap, event);
  CFRelease(event);
}
#endif // __APPLE__

#ifdef _WIN32
// Windows key code mapping from Linux KEY_* constants to Windows VK codes
static WORD winKeyCode(unsigned int linuxCode) {
  switch (linuxCode) {
    case KEY_LEFTCTRL:  return VK_CONTROL;
    case KEY_LEFTSHIFT: return VK_SHIFT;
    case KEY_A:         return 'A';
    case KEY_V:         return 'V';
    default:            return 0;
  }
}

bool KeyType::initKeyboard() {
  fprintf(stderr, "KeyType: Using SendInput keyboard backend\n");
  return true;
}

void KeyType::closeKeyboard() {
  // Nothing to clean up for SendInput
}

void KeyType::sendKey(unsigned int code, bool press) {
  WORD vk = winKeyCode(code);
  if (!vk) return;

  INPUT ip = {};
  ip.type = INPUT_KEYBOARD;
  ip.ki.wVk = vk;
  if (!press) ip.ki.dwFlags = KEYEVENTF_KEYUP;
  SendInput(1, &ip, sizeof(ip));
}
#endif // _WIN32

// ---------------------------------------------------------------------------
// Cross-platform clipboard and typing logic
// ---------------------------------------------------------------------------

void KeyType::copyToClipboard(const QString &text) {
#ifdef __linux__
  QProcess *proc = new QProcess();
  proc->setProgram("wl-copy");
  proc->setArguments({text});
  proc->start();
  proc->waitForFinished(1000);
  proc->deleteLater();
#else
  // macOS and Windows: use Qt clipboard (no external tool needed)
  QClipboard *clipboard = QApplication::clipboard();
  if (clipboard) {
    clipboard->setText(text);
  }
#endif
}

void KeyType::selectAllAndPaste() {
#ifdef __linux__
  if (m_fd < 0) {
    fprintf(stderr, "KeyType: uinput not initialized\n");
    return;
  }
#endif

  fprintf(stderr, "KeyType: Select All (Ctrl+A)...\n");

  // Ctrl+A (Select All) — Cmd+A on macOS
  sendKey(KEY_LEFTCTRL, true);  // mapped to Cmd in macKeyCode
  SleepMs(10);
  sendKey(KEY_A, true);
  SleepMs(10);
  sendKey(KEY_A, false);
  SleepMs(10);
  sendKey(KEY_LEFTCTRL, false);

#ifdef __linux__
  // Sync
  struct input_event ev = {};
  ev.type = EV_SYN;
  ev.code = SYN_REPORT;
  ev.value = 0;
  write(m_fd, &ev, sizeof(ev));
#endif

  SleepMs(50); // 50ms delay

  fprintf(stderr, "KeyType: Paste (Ctrl+V)...\n");

  // Ctrl+V (Paste) — Cmd+V on macOS
  sendKey(KEY_LEFTCTRL, true);
  SleepMs(10);
  sendKey(KEY_V, true);
  SleepMs(10);
  sendKey(KEY_V, false);
  SleepMs(10);
  sendKey(KEY_LEFTCTRL, false);

#ifdef __linux__
  // Sync
  write(m_fd, &ev, sizeof(ev));
#endif

  fprintf(stderr, "KeyType: Select All + Paste complete\n");
}

KeyType *KeyType::instance() {
  if (!s_instance) {
    s_instance = new KeyType();
  }
  return s_instance;
}
