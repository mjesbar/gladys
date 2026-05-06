#ifndef KEYTYPE_H
#define KEYTYPE_H

#include <QObject>
#include <QString>
#include <QQueue>
#include <QProcess>

class KeyType : public QObject {
  Q_OBJECT

public:
  static KeyType *instance();

  void push(const QString &chunk);

private:
  explicit KeyType(QObject *parent = nullptr);
  ~KeyType() = default;

  void processQueue();
  QString extractNewChunk(const QString &newText);
  QString normalizeText(const QString &text);

  static KeyType *s_instance;

  QQueue<QString> m_queue;
  QProcess *m_process;
  bool m_processing;
  QString m_lastTyped;
};

#endif // KEYTYPE_H