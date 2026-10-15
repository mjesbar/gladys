// Gladysd Hub - Main controller managing all system components.

#ifndef GLADYSD_H
#define GLADYSD_H

#include <QObject>
#include <QThread>
#include <QString>
#include <QVector>
#include <QTimer>
#include <QElapsedTimer>

#include "ipc.hpp"

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

  KeyGrab *key_grab();
  KeyType *key_type();
  STT *stt();
  LLM *llm();
  IPCServer *ipc_server();

signals:
  void toggle_requested();
  void audio_level_updated(const QVector<double> &levels);

private:
  static Gladysd *s_instance;

  explicit Gladysd(QObject *parent = nullptr);
  ~Gladysd() override;

  void setup_connections();
  static void signal_handler(int signum);

  // IPC Module
  IPCServer *m_ipc_server;
  QString m_ipc_server_name;
  // KeyGrab Module
  KeyGrab *m_key_grab;
  void *m_display;
  // STT Module
  QThread *m_stt_thread;
  STT *m_stt;
  // LLM Module
  LLM *m_llm;
  // KeyType Module
  KeyType *m_key_type;
  QThread *m_key_type_thread;
  QElapsedTimer m_audio_throttle;
};

#endif // GLADYSD_H