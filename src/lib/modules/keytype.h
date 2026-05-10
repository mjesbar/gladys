// KeyType Module - Keyboard typing via clipboard paste.

#ifndef KEYTYPE_H
#define KEYTYPE_H

#include <QObject>
#include <QString>
#include <QQueue>
#include <linux/uinput.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>

class KeyType : public QObject {
  Q_OBJECT

public:
  static KeyType *instance();
  void reset();
  void push(const QString &chunk);
  void copyToClipboard(const QString &text);
  void paste();
  void selectAllAndPaste();

private:
  explicit KeyType(QObject *parent = nullptr);
  ~KeyType();

  bool initUinput();
  void closeUinput();
  void sendKey(unsigned int code, bool press);
  void sendText(const QString &text);

  void processQueue();
  QString extractNewChunk(const QString &newText);
  void runTyping(const QString &chunk);

  static KeyType *s_instance;

  int m_fd;
  QQueue<QString> m_queue;
  bool m_processing;
  QString m_lastTyped;
};

#endif // KEYTYPE_H