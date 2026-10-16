// Paste Module - Sends Ctrl+Shift+V (plain-text paste) to the focused window.

#include "paste.hpp"

#include <QApplication>
#include <QClipboard>
#include <QProcess>

#include "../log/log.hpp"

#ifdef __linux__
#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <linux/uinput.h>
#include <sys/ioctl.h>
#include <unistd.h>
#endif

#ifdef __APPLE__
#include <CoreGraphics/CoreGraphics.h>
#endif

#ifdef _WIN32
#include <windows.h>
#define SleepMs(ms) Sleep(ms)
#else
#include <unistd.h>
#define SleepMs(ms) usleep((ms) * 1000)
#endif

// Linux KEY_* codes, defined as fallback where <linux/uinput.h> is absent.
#ifndef KEY_LEFTCTRL
#define KEY_LEFTCTRL 29
#endif
#ifndef KEY_LEFTSHIFT
#define KEY_LEFTSHIFT 42
#endif
#ifndef KEY_V
#define KEY_V 47
#endif

Paste::Paste(QObject *parent)
    : QObject(parent)
#ifdef __linux__
      ,
      m_fd(-1)
#endif
{
}

Paste::~Paste() {
  qCInfo(lcPaste) << "stopped";
#ifdef __linux__
  if (m_fd >= 0) {
    ioctl(m_fd, UI_DEV_DESTROY);
    close(m_fd);
  }
#endif
}

// ---------------------------------------------------------------------------
// Linux (uinput)
// ---------------------------------------------------------------------------

#ifdef __linux__

void Paste::init() {
  m_fd = open("/dev/uinput", O_WRONLY);
  if (m_fd < 0) {
    m_fd = open("/dev/input/uinput", O_WRONLY);
  }
  if (m_fd < 0) {
    qCCritical(lcPaste) << "failed to open uinput:" << strerror(errno);
    return;
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
    qCCritical(lcPaste) << "failed to create uinput:" << strerror(errno);
    close(m_fd);
    m_fd = -1;
    return;
  }
  qCInfo(lcPaste) << "started, uinput device ready";
}

void Paste::send_key(unsigned int code, bool press) {
  if (m_fd < 0) {
    return;
  }
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

void Paste::copy_to_clipboard(const QString &text) {
  QProcess proc;
  proc.start("wl-copy", {text});
  proc.waitForFinished(1000);
}

#endif // __linux__

// ---------------------------------------------------------------------------
// macOS (CGEvent)
// ---------------------------------------------------------------------------

#ifdef __APPLE__

void Paste::init() { qCInfo(lcPaste) << "started"; }

void Paste::send_key(unsigned int code, bool press) {
  CGKeyCode keyCode;
  switch (code) {
  case KEY_LEFTCTRL:
    keyCode = 0x3B; // kVK_Control
    break;
  case KEY_LEFTSHIFT:
    keyCode = 0x38; // kVK_Shift
    break;
  case KEY_V:
    keyCode = 0x09; // kVK_ANSI_V
    break;
  default:
    return;
  }
  CGEventRef event = CGEventCreateKeyboardEvent(NULL, keyCode, press);
  if (!event) {
    return;
  }
  CGEventPost(kCGHIDEventTap, event);
  CFRelease(event);
}

void Paste::copy_to_clipboard(const QString &text) {
  QApplication::clipboard()->setText(text);
}

#endif // __APPLE__

// ---------------------------------------------------------------------------
// Windows (SendInput)
// ---------------------------------------------------------------------------

#ifdef _WIN32

void Paste::init() { qCInfo(lcPaste) << "started"; }

void Paste::send_key(unsigned int code, bool press) {
  WORD vk;
  switch (code) {
  case KEY_LEFTCTRL:
    vk = VK_CONTROL;
    break;
  case KEY_LEFTSHIFT:
    vk = VK_SHIFT;
    break;
  case KEY_V:
    vk = 'V';
    break;
  default:
    return;
  }
  INPUT ip = {};
  ip.type = INPUT_KEYBOARD;
  ip.ki.wVk = vk;
  if (!press) {
    ip.ki.dwFlags = KEYEVENTF_KEYUP;
  }
  SendInput(1, &ip, sizeof(ip));
}

void Paste::copy_to_clipboard(const QString &text) {
  QApplication::clipboard()->setText(text);
}

#endif // _WIN32

// ---------------------------------------------------------------------------
// Shared sequence: Ctrl+Shift+V
// ---------------------------------------------------------------------------

void Paste::send_paste() {
  qCInfo(lcPaste) << "sending Ctrl+Shift+V";

  send_key(KEY_LEFTCTRL, true);
  send_key(KEY_LEFTSHIFT, true);
  SleepMs(10);
  send_key(KEY_V, true);
  SleepMs(10);
  send_key(KEY_V, false);
  send_key(KEY_LEFTSHIFT, false);
  send_key(KEY_LEFTCTRL, false);
}
