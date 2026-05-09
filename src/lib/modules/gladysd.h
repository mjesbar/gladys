// Gladysd Hub - Main controller managing all system components.

#ifndef GLADYSD_H
#define GLADYSD_H

#include <QObject>
#include <QThread>
#include <QString>
#include <QVector>
#include <QTimer>
#include <QElapsedTimer>

#include "ipc.h"

class KeyGrab;
class KeyType;
class STT;
class LLM;

class Gladysd : public QObject {
  Q_OBJECT

public:
  static Gladysd *instance();

  bool init();
  void shutdown();

  KeyGrab *keyGrab() { return m_keyGrab; }
  KeyType *keyType() { return m_keyType; }
  STT *stt() { return m_stt; }
  LLM *llm() { return m_llm; }
  IPCServer *ipcServer() { return m_ipcServer; }

signals:
  void toggleRequested();
  void audioLevelUpdated(const QVector<double> &levels);

private:
  static Gladysd *s_instance;

  explicit Gladysd(QObject *parent = nullptr);
  ~Gladysd() override;

  void setupConnections();
  static void signalHandler(int signum);

  // IPC Module
  IPCServer *m_ipcServer;
  QString m_ipcServerName;
  // KeyGrab Module
  KeyGrab *m_keyGrab;
  void *m_display;
  // STT Module
  QThread *m_sttThread;
  STT *m_stt;
  // LLM Module
  LLM *m_llm;
  // KeyType Module
  KeyType *m_keyType;
  QThread *m_keyTypeThread;
  QElapsedTimer m_audioThrottle;
};

#endif // GLADYSD_H