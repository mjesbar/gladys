// KeyType Module - Keyboard typing via clipboard paste.
// Cross-platform: uinput (Linux), CGEvent (macOS), SendInput (Windows).

#include "keytype.h"

#include <QApplication>
#include <QClipboard>
#include <QCoreApplication>
#include <QEventLoop>
#include <QProcess>

#ifdef __linux__
#include <cstring>
#include <cerrno>
#endif

KeyType *KeyType::s_instance = nullptr;

KeyType::KeyType(QObject *parent)
    : QObject(parent), m_processing(false)
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
#ifdef __APPLE__
  sendKey(KEY_LEFTCTRL, true);  // mapped to Cmd in macKeyCode
  usleep(10000);
  sendKey(KEY_A, true);
  usleep(10000);
  sendKey(KEY_A, false);
  usleep(10000);
  sendKey(KEY_LEFTCTRL, false);
#else
  sendKey(KEY_LEFTCTRL, true);
  usleep(10000);
  sendKey(KEY_A, true);
  usleep(10000);
  sendKey(KEY_A, false);
  usleep(10000);
  sendKey(KEY_LEFTCTRL, false);
#endif

#ifdef __linux__
  // Sync
  struct input_event ev = {};
  ev.type = EV_SYN;
  ev.code = SYN_REPORT;
  ev.value = 0;
  write(m_fd, &ev, sizeof(ev));
#endif

  usleep(50000); // 50ms delay

  fprintf(stderr, "KeyType: Paste (Ctrl+V)...\n");

  // Ctrl+V (Paste) — Cmd+V on macOS
  sendKey(KEY_LEFTCTRL, true);
  usleep(10000);
  sendKey(KEY_V, true);
  usleep(10000);
  sendKey(KEY_V, false);
  usleep(10000);
  sendKey(KEY_LEFTCTRL, false);

#ifdef __linux__
  // Sync
  write(m_fd, &ev, sizeof(ev));
#endif

  fprintf(stderr, "KeyType: Select All + Paste complete\n");
}

void KeyType::paste() {
  // Ctrl+Shift+V (paste without formatting) — Cmd+Shift+V on macOS
  sendKey(KEY_LEFTSHIFT, true);
  sendKey(KEY_LEFTCTRL, true);
  usleep(10000);
  sendKey(KEY_V, true);
  usleep(10000);
  sendKey(KEY_V, false);
  usleep(10000);
  sendKey(KEY_LEFTCTRL, false);
  sendKey(KEY_LEFTSHIFT, false);

#ifdef __linux__
  // Sync
  struct input_event ev = {};
  ev.type = EV_SYN;
  ev.code = SYN_REPORT;
  ev.value = 0;
  write(m_fd, &ev, sizeof(ev));
#endif
}

void KeyType::sendText(const QString &text) {
#ifdef __linux__
  if (m_fd < 0) return;
#endif

  copyToClipboard(text);
  usleep(50000); // 50ms for clipboard

  paste();
  usleep(100000); // 100ms after paste
}

void KeyType::reset() {
  m_lastTyped.clear();
  m_queue.clear();
  m_processing = false;
}

KeyType *KeyType::instance() {
  if (!s_instance) {
    s_instance = new KeyType();
  }
  return s_instance;
}

QString KeyType::extractNewChunk(const QString &newText) {
  if (m_lastTyped.isEmpty()) {
    QString trimmed = newText.trimmed();
    m_lastTyped = newText;
    return trimmed;
  }

  int lastLen = m_lastTyped.length();
  if (newText.length() <= lastLen) {
    return QString();
  }

  QString diff = newText.mid(lastLen);
  diff = diff.trimmed();
  if (diff.isEmpty()) {
    return QString();
  }

  m_lastTyped = newText;
  return diff;
}

void KeyType::push(const QString &chunk) {
  QString newChunk = extractNewChunk(chunk);
  if (newChunk.isEmpty()) {
    return;
  }
  m_queue.enqueue(newChunk);
  processQueue();
}

void KeyType::runTyping(const QString &chunk) {
#ifdef __linux__
  if (m_fd < 0) {
    fprintf(stderr, "KeyType: uinput not initialized\n");
    m_processing = false;
    processQueue();
    return;
  }
#endif

  fprintf(stderr, "KeyType: Typing: '%s'\n", qPrintable(chunk));
  sendText(chunk);
  fprintf(stderr, "KeyType: Typing complete\n");

  m_processing = false;
  processQueue();
}

void KeyType::processQueue() {
  if (m_processing || m_queue.isEmpty()) {
    return;
  }

  m_processing = true;
  QString chunk = m_queue.dequeue();
  runTyping(chunk);
}
