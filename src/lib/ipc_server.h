#ifndef IPC_SERVER_HPP
#define IPC_SERVER_HPP

#include <QLocalServer>
#include <QLocalSocket>
#include <QObject>
#include <QString>
#include <QDebug>

class IpcServer : public QObject {
  Q_OBJECT

public:
  explicit IpcServer(const QString &serverName, QObject *parent = nullptr);
  ~IpcServer() override;

  bool start();
  void sendToggle();

signals:
  void toggleRequested();

private:
  void onNewConnection();

  QLocalServer *m_server;
  QString m_serverName;
};

class IpcClient : public QObject {
  Q_OBJECT

public:
  explicit IpcClient(const QString &serverName, QObject *parent = nullptr);

  bool sendToggle();

private:
  QString m_serverName;
};

#endif // IPC_SERVER_HPP