// KeyType Module - Keyboard typing via clipboard paste.

#include "keytype.h"

#include <QCoreApplication>
#include <QEventLoop>
#include <QProcess>

KeyType *KeyType::s_instance = nullptr;

KeyType::KeyType(QObject *parent) : QObject(parent), m_processing(false), m_fd(-1) {
  initUinput();
}

KeyType::~KeyType() {
  closeUinput();
}

bool KeyType::initUinput() {
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

void KeyType::closeUinput() {
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

void KeyType::copyToClipboard(const QString &text) {
  QProcess *proc = new QProcess();
  proc->setProgram("wl-copy");
  proc->setArguments({text});
  proc->start();
  proc->waitForFinished(1000);
  proc->deleteLater();
}

void KeyType::paste() {
  // Ctrl+Shift+V (paste without formatting)
  sendKey(KEY_LEFTSHIFT, true);
  sendKey(KEY_LEFTCTRL, true);
  usleep(10000);
  sendKey(KEY_V, true);
  usleep(10000);
  sendKey(KEY_V, false);
  usleep(10000);
  sendKey(KEY_LEFTCTRL, false);
  sendKey(KEY_LEFTSHIFT, false);

  // Sync
  struct input_event ev = {};
  ev.type = EV_SYN;
  ev.code = SYN_REPORT;
  ev.value = 0;
  write(m_fd, &ev, sizeof(ev));
}

void KeyType::sendText(const QString &text) {
  if (m_fd < 0) return;

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
  if (m_fd < 0) {
    fprintf(stderr, "KeyType: uinput not initialized\n");
    m_processing = false;
    processQueue();
    return;
  }

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