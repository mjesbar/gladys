// IPC Module - Local socket-based inter-process communication.

#ifndef IPC_H
#define IPC_H

#include <QObject>
#include <QLocalServer>
#include <QLocalSocket>
#include <QString>
#include <QVector>

class IPCServer : public QObject {
  Q_OBJECT

public:
  explicit IPCServer(const QString &serverName, QObject *parent = nullptr);
  ~IPCServer() override;

  bool start();

signals:
  void toggle_requested();
  void quit_requested();
  void audio_level_updated(const QVector<double> &levels);

private:
  void on_new_connection();

  QLocalServer *m_server;
  QString m_server_name;
};

class IPCClient : public QObject {
  Q_OBJECT

public:
  explicit IPCClient(const QString &serverName, QObject *parent = nullptr);

  bool send_toggle();
  bool send_quit();
  bool send_audio_levels(const QVector<double> &levels);

private:
  QString m_server_name;
};

#endif // IPC_H