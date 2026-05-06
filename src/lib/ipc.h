// IPC Server - Local socket-based inter-process communication for gladys.

#ifndef IPC_HPP
#define IPC_HPP

#include <QLocalServer>
#include <QLocalSocket>
#include <QObject>
#include <QString>
#include <QDebug>

class IPCServer : public QObject {
  Q_OBJECT

public:
  explicit IPCServer(const QString &serverName, QObject *parent = nullptr);
  ~IPCServer() override;

  bool start();

signals:
  void toggleRequested();
  void quitRequested();

private:
  void onNewConnection();

  QLocalServer *m_server;
  QString m_serverName;
};

class IPCClient : public QObject {
  Q_OBJECT

public:
  explicit IPCClient(const QString &serverName, QObject *parent = nullptr);

  bool sendToggle();
  bool sendQuit();

private:
  QString m_serverName;
};

#endif // IPC_HPP