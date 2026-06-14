// KeyType Module - Keyboard typing via clipboard paste.
// Cross-platform: uinput (Linux), CGEvent (macOS), SendInput (Windows).

#ifndef KEYTYPE_H
#define KEYTYPE_H

#include <QObject>
#include <QString>

#ifdef __linux__
#include <linux/uinput.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#endif

#ifdef __APPLE__
#include <CoreGraphics/CoreGraphics.h>
#endif

#ifdef _WIN32
#include <windows.h>
#endif

class KeyType : public QObject {
  Q_OBJECT

public:
  static KeyType *instance();
  void copyToClipboard(const QString &text);
  void selectAllAndPaste();

private:
  explicit KeyType(QObject *parent = nullptr);
  ~KeyType();

  bool initKeyboard();
  void closeKeyboard();
  void sendKey(unsigned int code, bool press);

  static KeyType *s_instance;

#ifdef __linux__
  int m_fd;
#else
  int m_dummy;
#endif
};

#endif // KEYTYPE_H
